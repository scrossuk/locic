#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TargetInfo.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		void createPrimitiveSizeOf(Module& module, const std::string& name, const std::vector<SEM::Type*>& templateArguments, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, ArgInfo::None());
			
			LOG(LOG_INFO, "Generating sizeof() for primitive type '%s'.",
				name.c_str());
			
			if (name == "value_lval") {
				// The size of a built-in lvalue is entirely dependent
				// on the size of its target type.
				function.getBuilder().CreateRet(genSizeOf(function, templateArguments.at(0)));
			} else {
				function.getBuilder().CreateRet(
					ConstantGenerator(module).getSize(module.getTargetInfo().getPrimitiveSizeInBytes(name)));
			}
		}
		
		bool isIntegerType(const std::string& name) {
			return name == "char" || name == "short" || name == "int" || name == "long" || name == "longlong";
		}
		
		bool isFloatType(const std::string& name) {
			return name == "float" || name == "double" || name == "longdouble";
		}
		
		bool isUnaryOp(const std::string& methodName) {
			return methodName == "implicitCopy" ||
				   methodName == "not" ||
				   methodName == "isZero" ||
				   methodName == "isPositive" ||
				   methodName == "isNegative" ||
				   methodName == "abs" ||
				   methodName == "address" ||
				   methodName == "deref" ||
				   methodName == "dissolve" ||
				   methodName == "move";
		}
		
		bool isBinaryOp(const std::string& methodName) {
			return methodName == "add" ||
				   methodName == "subtract" ||
				   methodName == "multiply" ||
				   methodName == "divide" ||
				   methodName == "modulo" ||
				   methodName == "compare" ||
				   methodName == "assign" ||
				   methodName == "index";
		}
		
		ArgInfo getPrimitiveMethodArgInfo(const std::string& methodName) {
			assert((methodName == "Create") xor isUnaryOp(methodName) xor isBinaryOp(methodName));
			
			if (methodName == "Create") {
				return ArgInfo::None();
			}
			
			const bool hasReturnVarArg = false;
			const bool hasContextArg = true;
			const size_t numStandardArguments =
				isUnaryOp(methodName) ? 0 : 1;
			return ArgInfo(hasReturnVarArg, hasContextArg, numStandardArguments);
		}
		
		void createBoolPrimitiveMethod(Module& module, const std::string& methodName, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, getPrimitiveMethodArgInfo(methodName));
			
			llvm::IRBuilder<>& builder = function.getBuilder();
			
			llvm::Value* methodOwner =
				methodName == "Create" ?
					NULL :
					builder.CreateLoad(function.getContextValue());
			
			if (methodName == "Create") {
				builder.CreateRet(ConstantGenerator(module).getI1(false));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "not") {
					builder.CreateRet(builder.CreateNot(methodOwner));
				} else {
					throw std::runtime_error("Unknown bool unary op.");
				}
			} else {
				LOG(LOG_INFO, "Unknown bool method: %s.",
					methodName.c_str());
				throw std::runtime_error("Unknown bool method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createSignedIntegerPrimitiveMethod(Module& module, const std::string& typeName, const std::string& methodName, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, getPrimitiveMethodArgInfo(methodName));
			
			llvm::IRBuilder<>& builder = function.getBuilder();
			
			llvm::Value* methodOwner =
				methodName == "Create" ?
					NULL :
					builder.CreateLoad(function.getContextValue());
			
			if (methodName == "Create") {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
				builder.CreateRet(zero);
			} else if (isUnaryOp(methodName)) {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
				
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "isZero") {
					builder.CreateRet(builder.CreateICmpEQ(methodOwner, zero));
				} else if (methodName == "isPositive") {
					builder.CreateRet(builder.CreateICmpSGT(methodOwner, zero));
				} else if (methodName == "isNegative") {
					builder.CreateRet(builder.CreateICmpSLT(methodOwner, zero));
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					llvm::Value* lessThanZero = builder.CreateICmpSLT(methodOwner, zero);
					builder.CreateRet(
						builder.CreateSelect(lessThanZero, builder.CreateNeg(methodOwner), methodOwner));
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				llvm::Value* operand = function.getArg(0);
				
				if (methodName == "add") {
					builder.CreateRet(
						builder.CreateAdd(methodOwner, operand));
				} else if (methodName == "subtract") {
					builder.CreateRet(
						builder.CreateSub(methodOwner, operand));
				} else if (methodName == "multiply") {
					builder.CreateRet(
						builder.CreateMul(methodOwner, operand));
				} else if (methodName == "divide") {
					builder.CreateRet(
						builder.CreateSDiv(methodOwner, operand));
				} else if (methodName == "modulo") {
					builder.CreateRet(
						builder.CreateSRem(methodOwner, operand));
				} else if (methodName == "compare") {
					llvm::Value* isLessThan = builder.CreateICmpSLT(methodOwner, operand);
					llvm::Value* isGreaterThan = builder.CreateICmpSGT(methodOwner, operand);
					llvm::Value* minusOne = ConstantGenerator(module).getPrimitiveInt("int", -1);
					llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt("int", 0);
					llvm::Value* plusOne = ConstantGenerator(module).getPrimitiveInt("int", 1);
					llvm::Value* returnValue =
						builder.CreateSelect(isLessThan, minusOne,
							builder.CreateSelect(isGreaterThan, plusOne, zero));
					builder.CreateRet(returnValue);
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				LOG(LOG_INFO, "Unknown primitive method: %s::%s.",
					typeName.c_str(), methodName.c_str());
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createFloatPrimitiveMethod(Module& module, const std::string& typeName, const std::string& methodName, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, getPrimitiveMethodArgInfo(methodName));
			
			llvm::IRBuilder<>& builder = function.getBuilder();
			
			llvm::Value* methodOwner =
				methodName == "Create" ?
					NULL :
					builder.CreateLoad(function.getContextValue());
			
			if (methodName == "Create") {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
				builder.CreateRet(zero);
			} else if (isUnaryOp(methodName)) {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
				
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "isZero") {
					builder.CreateRet(builder.CreateFCmpOEQ(methodOwner, zero));
				} else if (methodName == "isPositive") {
					builder.CreateRet(builder.CreateFCmpOGT(methodOwner, zero));
				} else if (methodName == "isNegative") {
					builder.CreateRet(builder.CreateFCmpOLT(methodOwner, zero));
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					llvm::Value* lessThanZero = builder.CreateFCmpOLT(methodOwner, zero);
					builder.CreateRet(
						builder.CreateSelect(lessThanZero, builder.CreateFNeg(methodOwner), methodOwner));
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				llvm::Value* operand = function.getArg(0);
				
				if (methodName == "add") {
					builder.CreateRet(
						builder.CreateFAdd(methodOwner, operand));
				} else if (methodName == "subtract") {
					builder.CreateRet(
						builder.CreateFSub(methodOwner, operand));
				} else if (methodName == "multiply") {
					builder.CreateRet(
						builder.CreateFMul(methodOwner, operand));
				} else if (methodName == "divide") {
					builder.CreateRet(
						builder.CreateFDiv(methodOwner, operand));
				} else if (methodName == "modulo") {
					builder.CreateRet(
						builder.CreateFRem(methodOwner, operand));
				} else if (methodName == "compare") {
					llvm::Value* isLessThan = builder.CreateFCmpOLT(methodOwner, operand);
					llvm::Value* isGreaterThan = builder.CreateFCmpOGT(methodOwner, operand);
					llvm::Value* minusOne = ConstantGenerator(module).getPrimitiveInt("int", -1);
					llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt("int", 0);
					llvm::Value* plusOne = ConstantGenerator(module).getPrimitiveInt("int", 1);
					llvm::Value* returnValue =
						builder.CreateSelect(isLessThan, minusOne,
							builder.CreateSelect(isGreaterThan, plusOne, zero));
					builder.CreateRet(returnValue);
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				LOG(LOG_INFO, "Unknown primitive method: %s::%s.",
					typeName.c_str(), methodName.c_str());
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPtrPrimitiveMethod(Module& module, SEM::Type* parent, const std::string& methodName, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto targetType = parent->templateArguments().at(0);
			
			Function function(module, llvmFunction, getPrimitiveMethodArgInfo(methodName));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = builder.CreateLoad(function.getContextValue());
			
			if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "deref") {
					builder.CreateRet(methodOwner);
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				// TODO: implement addition and subtraction.
				const auto operand = function.getArg(0);
				
				if (methodName == "index") {
					const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
					const auto targetSize = genSizeOf(function, targetType);
					const auto offset = builder.CreateIntCast(operand, getPrimitiveType(module, "size_t", std::vector<llvm::Type*>()), true);
					const auto adjustedOffset = builder.CreateMul(offset, targetSize);
					const auto i8IndexPtr = builder.CreateGEP(i8BasePtr, adjustedOffset);
					const auto castPtr = builder.CreatePointerCast(i8IndexPtr, methodOwner->getType());
					builder.CreateRet(castPtr);
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				LOG(LOG_INFO, "Unknown primitive method: ptr::%s.", methodName.c_str());
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPtrLvalPrimitiveMethod(Module& module, SEM::Type* parent, const std::string& methodName, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto targetType = parent->templateArguments().at(0);
			
			Function function(module, llvmFunction, getPrimitiveMethodArgInfo(methodName));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = builder.CreateLoad(function.getContextValue());
			
			if (isUnaryOp(methodName)) {
				if (methodName == "address") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "dissolve") {
					builder.CreateRet(methodOwner);
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "assign") {
					genStore(function, operand, methodOwner, targetType);
					builder.CreateRetVoid();
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				LOG(LOG_INFO, "Unknown primitive method: ptr::%s.", methodName.c_str());
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		ArgInfo getValueLvalMethodArgInfo(Module& module, SEM::Type* targetType, const std::string& methodName) {
			if (methodName == "move") {
				const bool hasReturnVarArg = !isTypeSizeAlwaysKnown(module, targetType);
				const bool hasContextArg = true;
				const size_t numStandardArguments = 0;
				return ArgInfo(hasReturnVarArg, hasContextArg, numStandardArguments);
			}
		
			return getPrimitiveMethodArgInfo(methodName);
		}
		
		void genStoreValueLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::Type* unresolvedType) {
			assert(var->getType()->isPointerTy());
			
			auto& module = functionGenerator.getModule();
			
			const auto type = module.resolveType(unresolvedType);
			
			assert(type->isObject());
			
			const auto targetType = type->templateArguments().at(0);
			
			// Get a pointer to the liveness indicator.
			const auto livenessIndicator = functionGenerator.getBuilder().CreateConstInBoundsGEP2_32(var, 0, 0);
			
			// Get a pointer to the value, which is
			// just after the liveness indicator.
			const auto ptrToValue = functionGenerator.getBuilder().CreateConstInBoundsGEP2_32(var, 0, 1);
			
			// Set the liveness indicator.
			functionGenerator.getBuilder().CreateStore(ConstantGenerator(module).getI1(true), livenessIndicator);
			
			// Store the new child value.
			genStore(functionGenerator, value, ptrToValue, targetType);
		}
		
		void createValueLvalPrimitiveMethod(Module& module, SEM::Type* parent, const std::string& methodName, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto targetType = parent->templateArguments().at(0);
			
			Function function(module, llvmFunction, getValueLvalMethodArgInfo(module, targetType, methodName));
			
			auto& builder = function.getBuilder();
			
			// Get a pointer to the value, which is
			// just after the liveness indicator.
			const auto ptrToValue = builder.CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 1);
			
			if (isUnaryOp(methodName)) {
				if (methodName == "address") {
					builder.CreateRet(ptrToValue);
				} else if (methodName == "move") {
					// TODO: check liveness indicator (?).
					
					if (function.getArgInfo().hasReturnVarArgument()) {
						// For types where the size isn't always known
						// (i.e. non-primitives), a pointer to the return
						// value (the 'return var') will be passed, so
						// just store into that.
						genStore(function, ptrToValue, function.getReturnVar(), targetType);
						
						// Zero out the entire lval, which will
						// also reset the liveness indicator.
						genZero(function, parent, function.getContextValue());
						
						builder.CreateRetVoid();
					} else {
						// For types where the size is always known,
						// just load the value and return it.
						auto loadedValue = builder.CreateLoad(ptrToValue);
						
						// Zero out the entire lval, which will
						// also reset the liveness indicator.
						genZero(function, parent, function.getContextValue());
						
						builder.CreateRet(loadedValue);
					}
				} else if (methodName == "dissolve") {
					// TODO: check liveness indicator (?).
					builder.CreateRet(ptrToValue);
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "assign") {
					// Destroy any existing value.
					genDestructorCall(function, parent, function.getContextValue());
					
					// Get a pointer to the liveness indicator.
					const auto livenessIndicator = builder.CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 0);
					
					// Set the liveness indicator.
					builder.CreateStore(ConstantGenerator(module).getI1(true), livenessIndicator);
					
					// Store the new child value.
					genStore(function, operand, ptrToValue, targetType);
					
					builder.CreateRetVoid();
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				LOG(LOG_INFO, "Unknown primitive method: value_lval::%s.", methodName.c_str());
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPrimitiveMethod(Module& module, SEM::Type* parent, SEM::Function* function, llvm::Function& llvmFunction) {
			const std::string typeName = parent->getObjectType()->name().last();
			const std::string methodName = function->name().last();
			
			if (typeName == "bool") {
				createBoolPrimitiveMethod(module, methodName, llvmFunction);
			} else if (isIntegerType(typeName)) {
				createSignedIntegerPrimitiveMethod(module, typeName, methodName, llvmFunction);
			} else if (isFloatType(typeName)) {
				createFloatPrimitiveMethod(module, typeName, methodName, llvmFunction);
			} else if(typeName == "ptr") {
				createPtrPrimitiveMethod(module, parent, methodName, llvmFunction);
			} else if(typeName == "ptr_lval") {
				createPtrLvalPrimitiveMethod(module, parent, methodName, llvmFunction);
			} else if(typeName == "value_lval") {
				createValueLvalPrimitiveMethod(module, parent, methodName, llvmFunction);
			} else {
				throw std::runtime_error("TODO");
			}
		}
		
		void createValueLvalPrimitiveDestructor(Module& module, SEM::Type* parent, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto targetType = parent->templateArguments().at(0);
			
			Function function(module, llvmFunction, ArgInfo::ContextOnly());
			
			// Check the 'liveness indicator' which indicates whether
			// child value's destructor should be run.
			const auto isLive = function.getBuilder().CreateLoad(
				function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 0));
			
			const auto isLiveBB = function.createBasicBlock("is_live");
			const auto isNotLiveBB = function.createBasicBlock("is_not_live");
			
			function.getBuilder().CreateCondBr(isLive, isLiveBB, isNotLiveBB);
			
			// If it's not live, do nothing.
			function.selectBasicBlock(isNotLiveBB);
			function.getBuilder().CreateRetVoid();
			
			// If it is live, run the child value's destructor.
			function.selectBasicBlock(isLiveBB);
			const auto ptrToValue = function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 1);
			genDestructorCall(function, targetType, ptrToValue);
			function.getBuilder().CreateRetVoid();
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createVoidPrimitiveDestructor(Module& module, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, ArgInfo::ContextOnly());
			
			// Nothing to do; just return.
			function.getBuilder().CreateRetVoid();
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPrimitiveDestructor(Module& module, SEM::Type* parent, llvm::Function& llvmFunction) {
			const std::string typeName = parent->getObjectType()->name().last();
			if (typeName == "value_lval") {
				createValueLvalPrimitiveDestructor(module, parent, llvmFunction);
			} else {
				createVoidPrimitiveDestructor(module, llvmFunction);
			}
		}
		
		llvm::Type* getPrimitiveType(const Module& module, const std::string& name, const std::vector<llvm::Type*>& templateArguments) {
			if (name == "bool") {
				return TypeGenerator(module).getI1Type();
			}
			
			if (name == "char") {
				return TypeGenerator(module).getI8Type();
			}
			
			if (name == "short" || name == "int" || name == "long" || name == "longlong" || name == "size_t") {
				return TypeGenerator(module).getIntType(module.getTargetInfo().getPrimitiveSize(name));
			}
			
			if (name == "float") {
				return TypeGenerator(module).getFloatType();
			}
			
			if (name == "double") {
				return TypeGenerator(module).getDoubleType();
			}
			
			if (name == "longdouble") {
				return TypeGenerator(module).getLongDoubleType();
			}
			
			if (name == "ptr" || name == "ptr_lval") {
				assert(templateArguments.size() == 1);
				const auto targetType = templateArguments.at(0);
				if (targetType->isVoidTy()) {
					// LLVM doesn't support 'void *' => use 'int8_t *' instead.
					return TypeGenerator(module).getI8PtrType();
				} else {
					return targetType->getPointerTo();
				}
			}
			
			if (name == "value_lval") {
				assert(templateArguments.size() == 1);
				const auto targetType = templateArguments.at(0);
				
				std::vector<llvm::Type*> structVariables;
				
				// Add 'live' indicator (to determine whether child destructor is run).
				structVariables.push_back(TypeGenerator(module).getI1Type());
				
				// Add child value.
				structVariables.push_back(targetType);
				
				return TypeGenerator(module).getStructType(structVariables);
			}
			
			throw std::runtime_error("Unrecognised primitive type.");
		}
		
		bool primitiveTypeHasDestructor(Module& module, SEM::Type* type) {
			assert(type->isPrimitive());
			return type->getObjectType()->name().first() == "value_lval" && typeHasDestructor(module, type->templateArguments().at(0));
		}
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, SEM::Type* type) {
			assert(type->isPrimitive());
			return type->getObjectType()->name().first() != "value_lval" || isTypeSizeAlwaysKnown(module, type->templateArguments().at(0));
		}
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, SEM::Type* type) {
			assert(type->isPrimitive());
			return type->getObjectType()->name().first() != "value_lval" || isTypeSizeKnownInThisModule(module, type->templateArguments().at(0));
		}
		
	}
	
}

