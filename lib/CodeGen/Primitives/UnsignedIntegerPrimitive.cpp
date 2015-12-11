#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

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
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/UnsignedIntegerPrimitive.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				MethodID methodID, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		UnsignedIntegerPrimitive::UnsignedIntegerPrimitive(const SEM::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool UnsignedIntegerPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                               llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool UnsignedIntegerPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                                     llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool UnsignedIntegerPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool UnsignedIntegerPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type UnsignedIntegerPrimitive::getABIType(Module& /*module*/,
		                                                    const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                                    llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			switch (typeInstance_.primitiveID()) {
				case PrimitiveUInt8:
					return llvm_abi::UInt8Ty;
				case PrimitiveUInt16:
					return llvm_abi::UInt16Ty;
				case PrimitiveUInt32:
					return llvm_abi::UInt32Ty;
				case PrimitiveUInt64:
					return llvm_abi::UInt64Ty;
				case PrimitiveUByte:
					return llvm_abi::UCharTy;
				case PrimitiveUShort:
					return llvm_abi::UShortTy;
				case PrimitiveUInt:
					return llvm_abi::UIntTy;
				case PrimitiveULong:
					return llvm_abi::ULongTy;
				case PrimitiveULongLong:
					return llvm_abi::ULongLongTy;
				case PrimitiveSize:
					return llvm_abi::SizeTy;
				default:
					llvm_unreachable("Invalid unsigned integer primitive type.");
			}
		}
		
		llvm::Type* UnsignedIntegerPrimitive::getIRType(Module& module,
		                                                const TypeGenerator& typeGenerator,
		                                                llvm::ArrayRef<SEM::Value> templateArguments) const {
			const auto abiType = this->getABIType(module,
			                                      module.abiTypeBuilder(),
			                                      templateArguments);
			return typeGenerator.getIntType(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes() * 8);
		}
		
		llvm::Value* UnsignedIntegerPrimitive::emitMethod(IREmitter& irEmitter,
		                                                const MethodID methodID,
		                                                llvm::ArrayRef<SEM::Value> typeTemplateArguments,
		                                                llvm::ArrayRef<SEM::Value> functionTemplateArguments,
		                                                PendingResultArray args) const {
			auto& builder = irEmitter.builder();
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto& constantGenerator = irEmitter.constantGenerator();
			const auto& typeGenerator = irEmitter.typeGenerator();
			
			const auto primitiveID = typeInstance_.primitiveID();
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function);
			
			const bool unsafe = module.buildOptions().unsafe;
			const auto abiType = this->getABIType(module,
			                                      module.abiTypeBuilder(),
			                                      typeTemplateArguments);
			const size_t selfWidth = module.abi().typeInfo().getTypeAllocSize(abiType).asBytes() * 8;
			const auto selfType = typeGenerator.getIntType(selfWidth);
			const auto zero = constantGenerator.getPrimitiveInt(primitiveID, 0);
			const auto unit = constantGenerator.getPrimitiveInt(primitiveID, 1);
			
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
						const auto maxValue = constantGenerator.getSizeTValue(selfWidth);
						const auto exceedsMax = builder.CreateICmpUGT(operand, maxValue);
						const auto exceedsMaxBB = function.createBasicBlock("exceedsMax");
						const auto doesNotExceedMaxBB = function.createBasicBlock("doesNotExceedMax");
						builder.CreateCondBr(exceedsMax, exceedsMaxBB, doesNotExceedMaxBB);
						function.selectBasicBlock(exceedsMaxBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(doesNotExceedMaxBB);
					}
					
					const bool isSigned = false;
					
					const auto maxValue = constantGenerator.getSizeTValue(selfWidth);
					const auto shift = builder.CreateSub(maxValue, operand);
					
					// Use a 128-bit integer type to avoid overflow.
					const auto one128Bit = constantGenerator.getInt(128, 1);
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
						const auto maxValue = constantGenerator.getSizeTValue(selfWidth);
						const auto exceedsMax = builder.CreateICmpUGT(operand, maxValue);
						const auto exceedsMaxBB = function.createBasicBlock("exceedsMax");
						const auto doesNotExceedMaxBB = function.createBasicBlock("doesNotExceedMax");
						builder.CreateCondBr(exceedsMax, exceedsMaxBB, doesNotExceedMaxBB);
						function.selectBasicBlock(exceedsMaxBB);
						callTrapIntrinsic(function);
						function.selectBasicBlock(doesNotExceedMaxBB);
					}
					
					// Use a 128-bit integer type to avoid overflow.
					const auto one128Bit = constantGenerator.getInt(128, 1);
					const bool isSigned = false;
					const auto operandCasted = builder.CreateIntCast(operand, one128Bit->getType(), isSigned);
					const auto shiftedValue = builder.CreateShl(one128Bit, operandCasted);
					const auto result = builder.CreateSub(shiftedValue, one128Bit);
					return builder.CreateIntCast(result, selfType, isSigned);
				}
				case METHOD_IMPLICITCASTFROM:
				case METHOD_CASTFROM: {
					const auto argPrimitiveID = methodID.primitiveID();
					const auto operand = args[0].resolve(function);
					if (argPrimitiveID.isFloat()) {
						return builder.CreateFPToUI(operand, selfType);
					} else if (argPrimitiveID.isInteger()) {
						return builder.CreateZExtOrTrunc(operand, selfType);
					} else {
						llvm_unreachable("Unknown unsigned integer cast source type.");
					}
				}
				case METHOD_ALIGNMASK: {
					return constantGenerator.getSizeTValue(module.abi().typeInfo().getTypeRequiredAlign(abiType).asBytes() - 1);
				}
				case METHOD_SIZEOF: {
					return constantGenerator.getSizeTValue(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes());
				}
				case METHOD_IMPLICITCAST:
				case METHOD_CAST: {
					SEM::ValueArray valueArray;
					for (const auto& value: typeTemplateArguments) {
						valueArray.push_back(value.copy());
					}
					const auto type = SEM::Type::Object(&typeInstance_,
					                                    std::move(valueArray));
					return callCastMethod(function,
					                      methodOwner,
					                      type,
					                      methodID,
					                      functionTemplateArguments.front().typeRefType(),
					                      irEmitter.hintResultValue());
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return methodOwner;
				case METHOD_ISZERO:
					return irEmitter.emitI1ToBool(builder.CreateICmpEQ(methodOwner, zero));
				case METHOD_SIGNEDVALUE: {
					return methodOwner;
				}
				case METHOD_COUNTLEADINGZEROES: {
					const auto bitCount = countLeadingZeroes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, typeGenerator.getSizeTType(), isSigned);
				}
				case METHOD_COUNTLEADINGONES: {
					const auto bitCount = countLeadingOnes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, typeGenerator.getSizeTType(), isSigned);
				}
				case METHOD_COUNTTRAILINGZEROES: {
					const auto bitCount = countTrailingZeroes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, typeGenerator.getSizeTType(), isSigned);
				}
				case METHOD_COUNTTRAILINGONES: {
					const auto bitCount = countTrailingOnes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, typeGenerator.getSizeTType(), isSigned);
				}
				case METHOD_INCREMENT: {
					// TODO: add safety checks!
					const auto methodOwnerPtr = args[0].resolve(function);
					const auto incrementedValue = builder.CreateAdd(methodOwner, unit);
					irEmitter.emitRawStore(incrementedValue, methodOwnerPtr);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_DECREMENT: {
					// TODO: add safety checks!
					const auto methodOwnerPtr = args[0].resolve(function);
					const auto decrementedValue = builder.CreateSub(methodOwner, unit);
					irEmitter.emitRawStore(decrementedValue, methodOwnerPtr);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_SETDEAD: {
					// Do nothing.
					return constantGenerator.getVoidUndef();
				}
				case METHOD_ISLIVE: {
					return constantGenerator.getBool(true);
				}
				
				case METHOD_ADD: {
					const auto operand = args[1].resolveWithoutBind(function);
					llvm::Value* const binaryArgs[] = { methodOwner, operand };
					if (unsafe) {
						return builder.CreateAdd(methodOwner,
						                         operand,
						                         /*name=*/"",
						                         /*hasNUW=*/true);
					} else {
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::uadd_with_overflow, binaryArgs);
					}
				}
				case METHOD_SUBTRACT: {
					const auto operand = args[1].resolveWithoutBind(function);
					llvm::Value* const binaryArgs[] = { methodOwner, operand };
					if (unsafe) {
						return builder.CreateSub(methodOwner,
						                         operand,
						                         /*name=*/"",
						                         /*hasNUW=*/true);
					} else {
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::usub_with_overflow, binaryArgs);
					}
				}
				case METHOD_MULTIPLY: {
					const auto operand = args[1].resolveWithoutBind(function);
					if (unsafe) {
						return builder.CreateMul(methodOwner,
						                         operand,
						                         /*name=*/"",
						                         /*hasNUW=*/true);
					} else {
						const auto checkDivBB = function.createBasicBlock("");
						const auto trapBB = function.createBasicBlock("");
						const auto endBB = function.createBasicBlock("");
						
						const auto mulResult = builder.CreateMul(methodOwner,
						                                         operand);
						
						// Check if methodOwner == 0.
						const auto methodOwnerIsZero = builder.CreateICmpEQ(methodOwner,
						                                                    zero);
						builder.CreateCondBr(methodOwnerIsZero,
						                     endBB,
						                     checkDivBB);
						
						// If methodOwner != 0, check (mulResult / methodOwner) == operand.
						function.selectBasicBlock(checkDivBB);
						const auto divResult = builder.CreateUDiv(mulResult,
						                                          methodOwner);
						const auto divResultIsOperand = builder.CreateICmpEQ(divResult,
						                                                     operand);
						builder.CreateCondBr(divResultIsOperand,
						                     endBB,
						                     trapBB);
						
						// (mulResult / methodOwner) != operand -> trap.
						function.selectBasicBlock(trapBB);
						callTrapIntrinsic(function);
						
						function.selectBasicBlock(endBB);
						return mulResult;
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
					const auto minusOneResult = constantGenerator.getI8(-1);
					const auto zeroResult = constantGenerator.getI8(0);
					const auto plusOneResult = constantGenerator.getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpEQ(methodOwner, operand));
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpNE(methodOwner, operand));
				}
				case METHOD_LESSTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpULT(methodOwner, operand));
				}
				case METHOD_LESSTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpULE(methodOwner, operand));
				}
				case METHOD_GREATERTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpUGT(methodOwner, operand));
				}
				case METHOD_GREATERTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpUGE(methodOwner, operand));
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
						const auto maxValue = constantGenerator.getSizeTValue(selfWidth - 1);
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
					
					const auto destPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                               moveToPtr,
					                                               moveToPosition);
					irEmitter.emitRawStore(methodOwner, destPtr);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_INRANGE: {
					const auto leftOperand = args[1].resolve(function);
					const auto rightOperand = args[2].resolve(function);
					
					return irEmitter.emitI1ToBool(builder.CreateAnd(
						builder.CreateICmpULE(leftOperand, methodOwner),
						builder.CreateICmpULE(methodOwner, rightOperand)
					));
				}
				default:
					llvm_unreachable("Unknown unsigned integer primitive method.");
			}
		}
		
	}
	
}

