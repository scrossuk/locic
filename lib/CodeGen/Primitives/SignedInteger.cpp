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
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		bool isFloatType(Module& module, const SEM::Type* const rawType);
		
		bool isUnaryOp(const String& methodName);
		
		bool isBinaryOp(const String& methodName);
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				const String& methodName, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		llvm::Value* genSignedIntegerPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::FunctionType functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function);
			
			const bool unsafe = module.buildOptions().unsafe;
			const size_t selfWidth = module.abi().typeSize(genABIType(module, type)) * 8;
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(typeName, 1);
			
			if (methodID == METHOD_ALIGNMASK) {
				return ConstantGenerator(module).getSizeTValue(module.abi().typeAlign(genABIType(module, type)) - 1);
			} else if (methodID == METHOD_SIZEOF) {
				return ConstantGenerator(module).getSizeTValue(module.abi().typeSize(genABIType(module, type)));
			} else if (methodID == METHOD_IMPLICITCOPY || methodID == METHOD_COPY) {
				return methodOwner;
			} else if (methodName == "__move_to") {
				const auto moveToPtr = args[1].resolve(function);
				const auto moveToPosition = args[2].resolve(function);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genMoveStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "create") {
				return zero;
			} else if (methodName == "unit") {
				return unit;
			} else if (methodName == "__setdead" || methodName == "__set_dead") {
				// Do nothing.
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName.starts_with("implicit_cast_") || methodName.starts_with("cast_")) {
				const auto argType = functionType.parameterTypes().front();
				const auto operand = args[0].resolve(function);
				if (isFloatType(module, argType)) {
					return builder.CreateFPToSI(operand, selfType);
				} else {
					return builder.CreateSExtOrTrunc(operand, selfType);
				}
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicit_cast" || methodName == "cast") {
					return callCastMethod(function, methodOwner, type, methodName, templateArgs.front().typeRefType(), hintResultValue);
				} else if (methodName == "plus") {
					return methodOwner;
				} else if (methodName == "minus") {
					return builder.CreateNeg(methodOwner);
				} else if (methodName == "isZero") {
					return builder.CreateICmpEQ(methodOwner, zero);
				} else if (methodName == "isPositive") {
					return builder.CreateICmpSGT(methodOwner, zero);
				} else if (methodName == "isNegative") {
					return builder.CreateICmpSLT(methodOwner, zero);
				} else if (methodName == "unsigned_value") {
					return methodOwner;
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					const auto lessThanZero = builder.CreateICmpSLT(methodOwner, zero);
					return builder.CreateSelect(lessThanZero, builder.CreateNeg(methodOwner), methodOwner);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = args[1].resolveWithoutBind(function);
				llvm::Value* const binaryArgs[] = { methodOwner, operand };
				
				if (methodName == "add") {
					if (unsafe) {
						return builder.CreateAdd(methodOwner, operand);
					} else {
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::sadd_with_overflow, binaryArgs);
					}
				} else if (methodName == "subtract") {
					if (unsafe) {
						return builder.CreateSub(methodOwner, operand);
					} else {
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::ssub_with_overflow, binaryArgs);
					}
				} else if (methodName == "multiply") {
					if (unsafe) {
						return builder.CreateMul(methodOwner, operand);
					} else {
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::smul_with_overflow, binaryArgs);
					}
				} else if (methodName == "divide") {
					if (!unsafe) {
						// TODO: also check for case of MIN_INT / -1 leading to overflow.
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = function.createBasicBlock("isZero");
						const auto isNotZeroBB = function.createBasicBlock("isNotZero");
						builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
						function.selectBasicBlock(isZeroBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateSDiv(methodOwner, operand);
				} else if (methodName == "modulo") {
					if (!unsafe) {
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = function.createBasicBlock("isZero");
						const auto isNotZeroBB = function.createBasicBlock("isNotZero");
						builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
						function.selectBasicBlock(isZeroBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateSRem(methodOwner, operand);
				} else if (methodName == "equal") {
					return builder.CreateICmpEQ(methodOwner, operand);
				} else if (methodName == "not_equal") {
					return builder.CreateICmpNE(methodOwner, operand);
				} else if (methodName == "less_than") {
					return builder.CreateICmpSLT(methodOwner, operand);
				} else if (methodName == "less_than_or_equal") {
					return builder.CreateICmpSLE(methodOwner, operand);
				} else if (methodName == "greater_than") {
					return builder.CreateICmpSGT(methodOwner, operand);
				} else if (methodName == "greater_than_or_equal") {
					return builder.CreateICmpSGE(methodOwner, operand);
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateICmpSLT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpSGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
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

