#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		static llvm::Instruction* addDebugLoc(llvm::Instruction* instruction, const boost::optional<llvm::DebugLoc>& debugLocation) {
			if (debugLocation) {
				instruction->setDebugLoc(*debugLocation);
			}
			return instruction;
		}
		
		static llvm::Value* decodeReturnValue(Function& function, llvm::Value* value, llvm_abi::Type type, llvm::Type* llvmType) {
			std::vector<llvm_abi::Type> abiTypes;
			abiTypes.push_back(std::move(type));
			return function.module().abi().decodeValues(function.getEntryBuilder(), function.getBuilder(), {value}, abiTypes, {llvmType}).at(0);
		}
		
		llvm::Value* genFunctionCall(Function& function, llvm::Value* functionValue, llvm::Value* contextPointer,
				SEM::Type* functionType, const std::vector<SEM::Value*>& args, boost::optional<llvm::DebugLoc> debugLoc) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto returnType = functionType->getFunctionReturnType();
			const bool isTemplatedMethod = functionType->isFunctionTemplatedMethod();
			
			const auto functionPtr = isTemplatedMethod ? builder.CreateExtractValue(functionValue, { 0 }) : functionValue;
			const auto templateGenerator = isTemplatedMethod ? builder.CreateExtractValue(functionValue, { 1 }) : nullptr;
			
			const auto llvmFunctionType = functionPtr->getType()->getPointerElementType();
			assert(llvmFunctionType->isFunctionTy());
			
			std::vector<llvm::Value*> parameters;
			std::vector<llvm_abi::Type> parameterABITypes;
			
			// Some values (e.g. classes) will be returned
			// by assigning to a pointer passed as the first
			// argument (this deals with the class sizes
			// potentially being unknown).
			llvm::Value* returnVarValue = nullptr;
			
			if (!isTypeSizeAlwaysKnown(module, returnType)) {
				returnVarValue = genAlloca(function, returnType);
				parameters.push_back(function.getBuilder().CreatePointerCast(returnVarValue, TypeGenerator(module).getI8PtrType()));
				parameterABITypes.push_back(llvm_abi::Type::Pointer());
			}
			
			if (templateGenerator != nullptr) {
				parameters.push_back(templateGenerator);
				parameterABITypes.push_back(std::move(templateGeneratorType(module).first));
			}
			
			if (contextPointer != nullptr) {
				parameters.push_back(contextPointer);
				parameterABITypes.push_back(llvm_abi::Type::Pointer());
			}
			
			for (const auto param: args) {
				llvm::Value* argValue = genValue(function, param);
				llvm_abi::Type argABIType = genABIArgType(module, param->type());
				
				// When calling var-args functions, all 'char' and
				// 'short' values must be extended to 'int' values,
				// and all 'float' values must be converted to 'double'
				// values.
				if (llvmFunctionType->isFunctionVarArg() && param->type()->isPrimitive()) {
					const auto& typeName = param->type()->getObjectType()->name().last();
					
					const auto argType = argValue->getType();
					const auto sizeInBits = argType->getPrimitiveSizeInBits();
					
					if (argType->isIntegerTy() && sizeInBits < module.getTargetInfo().getPrimitiveSize("int_t")) {
						if (isSignedIntegerType(typeName)) {
							// Need to extend to int.
							argValue = function.getBuilder().CreateSExt(argValue, getNamedPrimitiveType(module, "int_t"));
							argABIType = llvm_abi::Type::Integer(llvm_abi::Int);
						} else if (isUnsignedIntegerType(typeName)) {
							// Need to extend to unsigned int.
							argValue = function.getBuilder().CreateZExt(argValue, getNamedPrimitiveType(module, "uint_t"));
							argABIType = llvm_abi::Type::Integer(llvm_abi::Int);
						}
					} else if (argType->isFloatingPointTy() && sizeInBits < 64) {
						// Need to extend to double.
						argValue = function.getBuilder().CreateFPExt(argValue, TypeGenerator(module).getDoubleType());
						argABIType = llvm_abi::Type::FloatingPoint(llvm_abi::Double);
					}
				}
				
				parameters.push_back(argValue);
				parameterABITypes.push_back(std::move(argABIType));
			}
			
			const auto encodedParameters = module.abi().encodeValues(function.getEntryBuilder(), function.getBuilder(), parameters, parameterABITypes);
			
			llvm::Value* encodedCallReturnValue = nullptr;
			
			if (!functionType->isFunctionNoExcept()) {
				const auto successPath = function.createBasicBlock("successPath");
				const auto failPath = function.createBasicBlock("failPath");
				
				encodedCallReturnValue = addDebugLoc(function.getBuilder().CreateInvoke(functionPtr, successPath, failPath, encodedParameters), debugLoc);
				
				// Fail path.
				function.selectBasicBlock(failPath);
				const bool isRethrow = false;
				genLandingPad(function, isRethrow);
				
				// Success path.
				function.selectBasicBlock(successPath);
			} else {
				encodedCallReturnValue = addDebugLoc(function.getBuilder().CreateCall(functionPtr, encodedParameters), debugLoc);
			}
			
			if (returnVarValue != nullptr) {
				// As above, if the return value pointer is used,
				// this should be loaded (and used instead).
				return genLoad(function, returnVarValue, returnType);
			} else {
				return decodeReturnValue(function, encodedCallReturnValue, genABIType(function.module(), returnType), genType(function.module(), returnType));
			}
		}
		
		llvm::Value* genRawFunctionCall(Function& function, const ArgInfo& argInfo, bool canThrow, llvm::Value* functionPtr,
				const std::vector<llvm::Value*>& args, boost::optional<llvm::DebugLoc> debugLoc) {
			
			assert(args.size() == argInfo.argumentTypes().size());
			
			const auto llvmFunctionType = functionPtr->getType()->getPointerElementType();
			assert(llvmFunctionType->isFunctionTy());
			
			std::vector<llvm_abi::Type> argABITypes;
			for (const auto& typePair: argInfo.argumentTypes()) {
				argABITypes.push_back(typePair.first.copy());
			}
			
			// Parameters need to be encoded according to the ABI.
			const auto encodedParameters = function.module().abi().encodeValues(function.getEntryBuilder(), function.getBuilder(), args, argABITypes);
			
			llvm::Value* encodedCallReturnValue = nullptr;
			
			if (canThrow) {
				const auto successPath = function.createBasicBlock("successPath");
				const auto failPath = function.createBasicBlock("failPath");
				
				encodedCallReturnValue = addDebugLoc(function.getBuilder().CreateInvoke(functionPtr, successPath, failPath, encodedParameters), debugLoc);
				
				// Fail path.
				function.selectBasicBlock(failPath);
				const bool isRethrow = false;
				genLandingPad(function, isRethrow);
				
				// Success path.
				function.selectBasicBlock(successPath);
			} else {
				const auto callInst = function.getBuilder().CreateCall(functionPtr, encodedParameters);
				callInst->setDoesNotThrow();
				encodedCallReturnValue = addDebugLoc(callInst, debugLoc);
			}
			
			// Return values need to be decoded according to the ABI.
			return decodeReturnValue(function, encodedCallReturnValue, argInfo.returnType().first.copy(), argInfo.returnType().second);
		}
		
	}
	
}

