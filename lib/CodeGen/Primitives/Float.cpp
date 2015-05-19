#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		bool isFloatType(Module& module, const SEM::Type* const rawType);
		
		bool isUnaryOp(const String& methodName);
		
		bool isBinaryOp(const String& methodName);
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				const String& methodName, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		llvm::Value* genFloatPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::FunctionType functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function);
			
			if (methodName == "__move_to") {
				const auto moveToPtr = args[1].resolve(function);
				const auto moveToPosition = args[2].resolve(function);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genMoveStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "create") {
				return ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
			} else if (methodName == "__setdead" || methodName == "__set_dead") {
				// Do nothing.
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "__islive" || methodName == "__is_live") {
				return ConstantGenerator(module).getI1(true);
			} else if (methodName.starts_with("implicit_cast_") || methodName.starts_with("cast_")) {
				const auto argType = functionType.parameterTypes().front();
				const auto operand = args[0].resolve(function);
				const auto selfType = genType(module, type);
				if (isFloatType(module, argType)) {
					if (methodName.starts_with("implicit_cast_")) {
						return builder.CreateFPExt(operand, selfType);
					} else {
						return builder.CreateFPTrunc(operand, selfType);
					}
				} else if (isUnsignedIntegerType(module, argType)) {
					return builder.CreateUIToFP(operand, selfType);
				} else if (isSignedIntegerType(module, argType)) {
					return builder.CreateSIToFP(operand, selfType);
				} else {
					llvm_unreachable("Unknown float cast source type.");
				}
			} else if (isUnaryOp(methodName)) {
				const auto zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
				
				if (methodName == "implicit_cast" || methodName == "cast") {
					return callCastMethod(function, methodOwner, type, methodName, templateArgs.front().typeRefType(), hintResultValue);
				} else if (methodName == "implicit_copy" || methodName == "copy" || methodName == "plus") {
					return methodOwner;
				} else if (methodName == "minus") {
					return builder.CreateFNeg(methodOwner);
				} else if (methodName == "isZero") {
					return builder.CreateFCmpOEQ(methodOwner, zero);
				} else if (methodName == "isPositive") {
					return builder.CreateFCmpOGT(methodOwner, zero);
				} else if (methodName == "isNegative") {
					return builder.CreateFCmpOLT(methodOwner, zero);
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					const auto lessThanZero = builder.CreateFCmpOLT(methodOwner, zero);
					return builder.CreateSelect(lessThanZero, builder.CreateFNeg(methodOwner), methodOwner);
				} else if (methodName == "sqrt") {
					llvm::Type* const intrinsicTypes[] = { methodOwner->getType() };
					const auto sqrtIntrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::sqrt, intrinsicTypes);
					llvm::Value* const sqrtArgs[] = { methodOwner };
					return builder.CreateCall(sqrtIntrinsic, sqrtArgs);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = args[1].resolveWithoutBind(function);
				
				if (methodName == "add") {
					return builder.CreateFAdd(methodOwner, operand);
				} else if (methodName == "subtract") {
					return builder.CreateFSub(methodOwner, operand);
				} else if (methodName == "multiply") {
					return builder.CreateFMul(methodOwner, operand);
				} else if (methodName == "divide") {
					return builder.CreateFDiv(methodOwner, operand);
				} else if (methodName == "modulo") {
					return builder.CreateFRem(methodOwner, operand);
				} else if (methodName == "equal") {
					return builder.CreateFCmpOEQ(methodOwner, operand);
				} else if (methodName == "not_equal") {
					return builder.CreateFCmpONE(methodOwner, operand);
				} else if (methodName == "less_than") {
					return builder.CreateFCmpOLT(methodOwner, operand);
				} else if (methodName == "less_than_or_equal") {
					return builder.CreateFCmpOLE(methodOwner, operand);
				} else if (methodName == "greater_than") {
					return builder.CreateFCmpOGT(methodOwner, operand);
				} else if (methodName == "greater_than_or_equal") {
					return builder.CreateFCmpOGE(methodOwner, operand);
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateFCmpOLT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateFCmpOGT(methodOwner, operand);
					const auto minusOne = ConstantGenerator(module).getI8(-1);
					const auto zero = ConstantGenerator(module).getI8(0);
					const auto plusOne = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOne,
							builder.CreateSelect(isGreaterThan, plusOne, zero));
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				printf("%s\n", methodName.c_str());
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
	}
	
}

