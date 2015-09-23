#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
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
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				MethodID methodID, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		llvm::Value* genUnsignedIntegerPrimitiveMethodCall(Function& function, const SEM::Type* type, const MethodID methodID, const SEM::FunctionType functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function);
			
			const bool unsafe = module.buildOptions().unsafe;
			const size_t selfWidth = module.abi().typeSize(genABIType(module, type)) * 8;
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(type->primitiveID(), 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(type->primitiveID(), 1);
			
			switch (methodID) {
				case METHOD_CREATE:
				case METHOD_ZERO:
					return zero;
				case METHOD_UNIT:
					return unit;
				case METHOD_LEADINGONES: {
					const auto operand = args[0].resolve(function);
					
					if (!unsafe) {
						// Check that operand <= sizeof(type) * 8.
						const auto maxValue = ConstantGenerator(module).getSizeTValue(selfWidth);
						const auto exceedsMax = builder.CreateICmpUGT(operand, maxValue);
						const auto exceedsMaxBB = function.createBasicBlock("exceedsMax");
						const auto doesNotExceedMaxBB = function.createBasicBlock("doesNotExceedMax");
						builder.CreateCondBr(exceedsMax, exceedsMaxBB, doesNotExceedMaxBB);
						function.selectBasicBlock(exceedsMaxBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(doesNotExceedMaxBB);
					}
					
					const bool isSigned = false;
					
					const auto maxValue = ConstantGenerator(module).getSizeTValue(selfWidth);
					const auto shift = builder.CreateSub(maxValue, operand);
					
					// Use a 128-bit integer type to avoid overflow.
					const auto one128Bit = ConstantGenerator(module).getInt(128, 1);
					const auto shiftCasted = builder.CreateIntCast(shift, one128Bit->getType(), isSigned);
					const auto shiftedValue = builder.CreateShl(one128Bit, shiftCasted);
					const auto trailingOnesValue = builder.CreateSub(shiftedValue, one128Bit);
					const auto result = builder.CreateNot(trailingOnesValue);
					return builder.CreateIntCast(result, selfType, isSigned);
				}
				case METHOD_TRAILINGONES: {
					const auto operand = args[0].resolve(function);
					
					if (!unsafe) {
						// Check that operand <= sizeof(type) * 8.
						const auto maxValue = ConstantGenerator(module).getSizeTValue(selfWidth);
						const auto exceedsMax = builder.CreateICmpUGT(operand, maxValue);
						const auto exceedsMaxBB = function.createBasicBlock("exceedsMax");
						const auto doesNotExceedMaxBB = function.createBasicBlock("doesNotExceedMax");
						builder.CreateCondBr(exceedsMax, exceedsMaxBB, doesNotExceedMaxBB);
						function.selectBasicBlock(exceedsMaxBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(doesNotExceedMaxBB);
					}
					
					// Use a 128-bit integer type to avoid overflow.
					const auto one128Bit = ConstantGenerator(module).getInt(128, 1);
					const bool isSigned = false;
					const auto operandCasted = builder.CreateIntCast(operand, one128Bit->getType(), isSigned);
					const auto shiftedValue = builder.CreateShl(one128Bit, operandCasted);
					const auto result = builder.CreateSub(shiftedValue, one128Bit);
					return builder.CreateIntCast(result, selfType, isSigned);
				}
				case METHOD_IMPLICITCASTFROM:
				case METHOD_CASTFROM: {
					const auto argType = functionType.parameterTypes().front();
					const auto operand = args[0].resolve(function);
					if (isFloatType(module, argType)) {
						return builder.CreateFPToUI(operand, selfType);
					} else {
						return builder.CreateZExtOrTrunc(operand, selfType);
					}
				}
				case METHOD_ALIGNMASK: {
					return ConstantGenerator(module).getSizeTValue(module.abi().typeAlign(genABIType(module, type)) - 1);
				}
				case METHOD_SIZEOF: {
					return ConstantGenerator(module).getSizeTValue(module.abi().typeSize(genABIType(module, type)));
				}
					
				case METHOD_IMPLICITCAST:
				case METHOD_CAST:
					return callCastMethod(function, methodOwner, type, methodID, templateArgs.front().typeRefType(), hintResultValue);
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return methodOwner;
				case METHOD_ISZERO:
					return builder.CreateICmpEQ(methodOwner, zero);
				case METHOD_SIGNEDVALUE: {
					return methodOwner;
				}
				case METHOD_COUNTLEADINGZEROES: {
					const auto bitCount = countLeadingZeroes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, TypeGenerator(module).getSizeTType(), isSigned);
				}
				case METHOD_COUNTLEADINGONES: {
					const auto bitCount = countLeadingOnes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, TypeGenerator(module).getSizeTType(), isSigned);
				}
				case METHOD_COUNTTRAILINGZEROES: {
					const auto bitCount = countTrailingZeroes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, TypeGenerator(module).getSizeTType(), isSigned);
				}
				case METHOD_COUNTTRAILINGONES: {
					const auto bitCount = countTrailingOnes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, TypeGenerator(module).getSizeTType(), isSigned);
				}
				case METHOD_SQRT:
				case METHOD_INCREMENT: {
					// TODO: add safety checks!
					const auto methodOwnerPtr = args[0].resolve(function);
					const auto incrementedValue = builder.CreateAdd(methodOwner, unit);
					builder.CreateStore(incrementedValue, methodOwnerPtr);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_DECREMENT: {
					// TODO: add safety checks!
					const auto methodOwnerPtr = args[0].resolve(function);
					const auto decrementedValue = builder.CreateSub(methodOwner, unit);
					builder.CreateStore(decrementedValue, methodOwnerPtr);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_SETDEAD: {
					// Do nothing.
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ISLIVE: {
					return ConstantGenerator(module).getI1(true);
				}
				
				case METHOD_ADD: {
					const auto operand = args[1].resolveWithoutBind(function);
					llvm::Value* const binaryArgs[] = { methodOwner, operand };
					if (unsafe) {
						return builder.CreateAdd(methodOwner, operand);
					} else {
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::uadd_with_overflow, binaryArgs);
					}
				}
				case METHOD_SUBTRACT: {
					const auto operand = args[1].resolveWithoutBind(function);
					llvm::Value* const binaryArgs[] = { methodOwner, operand };
					if (unsafe) {
						return builder.CreateSub(methodOwner, operand);
					} else {
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::usub_with_overflow, binaryArgs);
					}
				}
				case METHOD_MULTIPLY: {
					const auto operand = args[1].resolveWithoutBind(function);
					llvm::Value* const binaryArgs[] = { methodOwner, operand };
					if (unsafe) {
						return builder.CreateMul(methodOwner, operand);
					} else {
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::umul_with_overflow, binaryArgs);
					}
				}
				case METHOD_DIVIDE: {
					const auto operand = args[1].resolveWithoutBind(function);
					if (!unsafe) {
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = function.createBasicBlock("isZero");
						const auto isNotZeroBB = function.createBasicBlock("isNotZero");
						builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
						function.selectBasicBlock(isZeroBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateUDiv(methodOwner, operand);
				}
				case METHOD_MODULO: {
					const auto operand = args[1].resolveWithoutBind(function);
					if (!unsafe) {
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = function.createBasicBlock("isZero");
						const auto isNotZeroBB = function.createBasicBlock("isNotZero");
						builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
						function.selectBasicBlock(isZeroBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateURem(methodOwner, operand);
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpEQ(methodOwner, operand);
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpNE(methodOwner, operand);
				}
				case METHOD_LESSTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpULT(methodOwner, operand);
				}
				case METHOD_LESSTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpULE(methodOwner, operand);
				}
				case METHOD_GREATERTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpUGT(methodOwner, operand);
				}
				case METHOD_GREATERTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpUGE(methodOwner, operand);
				}
				case METHOD_BITWISEAND: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateAnd(methodOwner, operand);
				}
				case METHOD_BITWISEOR: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateOr(methodOwner, operand);
				}
				case METHOD_LEFTSHIFT: {
					const auto operand = args[1].resolveWithoutBind(function);
					
					if (!unsafe) {
						// Check that operand <= leading_zeroes(value).
						
						// Calculate leading zeroes, or produce sizeof(T) * 8 - 1 if value == 0
						// (which prevents shifting 0 by sizeof(T) * 8).
						const auto leadingZeroes = countLeadingZeroesBounded(function, methodOwner);
						
						const bool isSigned = false;
						const auto leadingZeroesSizeT = builder.CreateIntCast(leadingZeroes, operand->getType(), isSigned);
						
						const auto exceedsLeadingZeroes = builder.CreateICmpUGT(operand, leadingZeroesSizeT);
						const auto exceedsLeadingZeroesBB = function.createBasicBlock("exceedsLeadingZeroes");
						const auto doesNotExceedLeadingZeroesBB = function.createBasicBlock("doesNotExceedLeadingZeroes");
						builder.CreateCondBr(exceedsLeadingZeroes, exceedsLeadingZeroesBB, doesNotExceedLeadingZeroesBB);
						function.selectBasicBlock(exceedsLeadingZeroesBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(doesNotExceedLeadingZeroesBB);
					}
					
					const bool isSigned = false;
					const auto operandCasted = builder.CreateIntCast(operand, selfType, isSigned);
					
					return builder.CreateShl(methodOwner, operandCasted);
				}
				case METHOD_RIGHTSHIFT: {
					const auto operand = args[1].resolveWithoutBind(function);
					
					if (!unsafe) {
						// Check that operand < sizeof(type) * 8.
						const auto maxValue = ConstantGenerator(module).getSizeTValue(selfWidth - 1);
						const auto exceedsMax = builder.CreateICmpUGT(operand, maxValue);
						const auto exceedsMaxBB = function.createBasicBlock("exceedsMax");
						const auto doesNotExceedMaxBB = function.createBasicBlock("doesNotExceedMax");
						builder.CreateCondBr(exceedsMax, exceedsMaxBB, doesNotExceedMaxBB);
						function.selectBasicBlock(exceedsMaxBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(doesNotExceedMaxBB);
					}
					
					const bool isSigned = false;
					const auto operandCasted = builder.CreateIntCast(operand, selfType, isSigned);
					
					return builder.CreateLShr(methodOwner, operandCasted);
				}
				
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
					
					genMoveStore(function, methodOwner, castedDestPtr, type);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_INRANGE: {
					const auto leftOperand = args[1].resolve(function);
					const auto rightOperand = args[2].resolve(function);
					
					return builder.CreateAnd(
						builder.CreateICmpULE(leftOperand, methodOwner),
						builder.CreateICmpULE(methodOwner, rightOperand)
					);
				}
				default:
					llvm_unreachable("Unknown unsigned integer primitive method.");
			}
		}
		
	}
	
}

