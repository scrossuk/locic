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
		
		static llvm::Value* decodeReturnValue(Function& function, llvm::Value* value, llvm_abi::Type* type, llvm::Type* llvmType) {
			std::vector<llvm_abi::Type*> abiTypes;
			abiTypes.push_back(type);
			
			std::vector<llvm::Value*> values;
			values.push_back(value);
			function.module().abi().decodeValues(function.getEntryBuilder(), function.getBuilder(), values, abiTypes, {llvmType});
			return values.at(0);
		}
		
		llvm::Value* genFunctionCall(Function& function, FunctionCallInfo callInfo, SEM::Type* functionType, const std::vector<SEM::Value*>& args, boost::optional<llvm::DebugLoc> debugLoc) {
			assert(callInfo.functionPtr != nullptr);
			
			auto& module = function.module();
			auto& abiContext = module.abiContext();
			
			const auto returnType = functionType->getFunctionReturnType();
			
			const auto llvmFunctionType = callInfo.functionPtr->getType()->getPointerElementType();
			assert(llvmFunctionType->isFunctionTy());
			
			std::vector<llvm::Value*> parameters;
			parameters.reserve(3 + args.size());
			
			llvm::SmallVector<llvm_abi::Type*, 10> parameterABITypes;
			
			// Some values (e.g. classes) will be returned
			// by assigning to a pointer passed as the first
			// argument (this deals with the class sizes
			// potentially being unknown).
			llvm::Value* returnVarValue = nullptr;
			
			if (!isTypeSizeAlwaysKnown(module, returnType)) {
				returnVarValue = genAlloca(function, returnType);
				parameters.push_back(returnVarValue);
				parameterABITypes.push_back(llvm_abi::Type::Pointer(abiContext));
			}
			
			if (callInfo.templateGenerator != nullptr) {
				parameters.push_back(callInfo.templateGenerator);
				parameterABITypes.push_back(templateGeneratorType(module).first);
			}
			
			if (callInfo.contextPointer != nullptr) {
				parameters.push_back(callInfo.contextPointer);
				parameterABITypes.push_back(llvm_abi::Type::Pointer(abiContext));
			}
			
			for (const auto param: args) {
				llvm::Value* argValue = genValue(function, param);
				llvm_abi::Type* argABIType = genABIArgType(module, param->type());
				
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
							argABIType = llvm_abi::Type::Integer(abiContext, llvm_abi::Int);
						} else if (isUnsignedIntegerType(typeName)) {
							// Need to extend to unsigned int.
							argValue = function.getBuilder().CreateZExt(argValue, getNamedPrimitiveType(module, "uint_t"));
							argABIType = llvm_abi::Type::Integer(abiContext, llvm_abi::Int);
						}
					} else if (argType->isFloatingPointTy() && sizeInBits < 64) {
						// Need to extend to double.
						argValue = function.getBuilder().CreateFPExt(argValue, TypeGenerator(module).getDoubleType());
						argABIType = llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::Double);
					}
				}
				
				parameters.push_back(argValue);
				parameterABITypes.push_back(argABIType);
			}
			
			module.abi().encodeValues(function.getEntryBuilder(), function.getBuilder(), parameters, parameterABITypes);
			
			llvm::Value* encodedCallReturnValue = nullptr;
			
			const bool isRethrow = false;
			if (!functionType->isFunctionNoExcept() && anyExceptionActions(function, isRethrow)) {
				const auto successPath = function.createBasicBlock("");
				const auto failPath = genLandingPad(function, isRethrow);
				
				encodedCallReturnValue = addDebugLoc(function.getBuilder().CreateInvoke(callInfo.functionPtr, successPath, failPath, parameters), debugLoc);
				
				function.selectBasicBlock(successPath);
			} else {
				encodedCallReturnValue = addDebugLoc(function.getBuilder().CreateCall(callInfo.functionPtr, parameters), debugLoc);
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
				llvm::ArrayRef<llvm::Value*> args, boost::optional<llvm::DebugLoc> debugLoc) {
			
			assert(args.size() == argInfo.argumentTypes().size());
			
			assert(functionPtr->getType()->getPointerElementType()->isFunctionTy());
			
			llvm::SmallVector<llvm_abi::Type*, 10> argABITypes;
			argABITypes.reserve(argInfo.argumentTypes().size());
			
			for (const auto& typePair: argInfo.argumentTypes()) {
				argABITypes.push_back(typePair.first);
			}
			
			// Parameters need to be encoded according to the ABI.
			std::vector<llvm::Value*> encodedParameters = args.vec();
			function.module().abi().encodeValues(function.getEntryBuilder(), function.getBuilder(), encodedParameters, argABITypes);
			
			llvm::Value* encodedCallReturnValue = nullptr;
			
			const bool isRethrow = false;
			if (canThrow && anyExceptionActions(function, isRethrow)) {
				const auto successPath = function.createBasicBlock("");
				const auto failPath = genLandingPad(function, isRethrow);
				
				encodedCallReturnValue = addDebugLoc(function.getBuilder().CreateInvoke(functionPtr, successPath, failPath, encodedParameters), debugLoc);
				
				function.selectBasicBlock(successPath);
			} else {
				const auto callInst = function.getBuilder().CreateCall(functionPtr, encodedParameters);
				if (!canThrow) {
					callInst->setDoesNotThrow();
				}
				encodedCallReturnValue = addDebugLoc(callInst, debugLoc);
			}
			
			// Return values need to be decoded according to the ABI.
			return decodeReturnValue(function, encodedCallReturnValue, argInfo.returnType().first, argInfo.returnType().second);
		}
		
	}
	
}

