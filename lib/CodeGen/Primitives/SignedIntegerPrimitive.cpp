#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

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
#include <locic/CodeGen/Primitives/SignedIntegerPrimitive.hpp>
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
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const AST::Type* const castFromType,
				MethodID methodID, const AST::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		SignedIntegerPrimitive::SignedIntegerPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool SignedIntegerPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                               llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool SignedIntegerPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                                     llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool SignedIntegerPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool SignedIntegerPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type SignedIntegerPrimitive::getABIType(Module& /*module*/,
		                                                  const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                                  llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			switch (typeInstance_.primitiveID()) {
				case PrimitiveInt8:
					return llvm_abi::Int8Ty;
				case PrimitiveInt16:
					return llvm_abi::Int16Ty;
				case PrimitiveInt32:
					return llvm_abi::Int32Ty;
				case PrimitiveInt64:
					return llvm_abi::Int64Ty;
				case PrimitiveByte:
					return llvm_abi::CharTy;
				case PrimitiveShort:
					return llvm_abi::ShortTy;
				case PrimitiveInt:
					return llvm_abi::IntTy;
				case PrimitiveLong:
					return llvm_abi::LongTy;
				case PrimitiveLongLong:
					return llvm_abi::LongLongTy;
				case PrimitiveSSize:
					return llvm_abi::SSizeTy;
				case PrimitivePtrDiff:
					return llvm_abi::PtrDiffTy;
				default:
					llvm_unreachable("Invalid signed integer primitive type.");
			}
		}
		
		llvm::Type* SignedIntegerPrimitive::getIRType(Module& module,
		                                              const TypeGenerator& typeGenerator,
		                                              llvm::ArrayRef<AST::Value> templateArguments) const {
			const auto abiType = this->getABIType(module,
			                                      module.abiTypeBuilder(),
			                                      templateArguments);
			return typeGenerator.getIntType(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes() * 8);
		}
		
		llvm::Value* SignedIntegerPrimitive::emitMethod(IREmitter& irEmitter,
		                                                const MethodID methodID,
		                                                llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                                llvm::ArrayRef<AST::Value> functionTemplateArguments,
		                                                PendingResultArray args,
		                                                llvm::Value* const hintResultValue) const {
			auto& builder = irEmitter.builder();
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto& constantGenerator = irEmitter.constantGenerator();
			const auto& typeGenerator = irEmitter.typeGenerator();
			
			const auto primitiveID = typeInstance_.primitiveID();
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function);
			
			const bool unsafe = module.buildOptions().unsafe;
			const auto zero = constantGenerator.getPrimitiveInt(primitiveID, 0);
			
			switch (methodID) {
				case METHOD_ALIGNMASK: {
					const auto abiType = this->getABIType(module,
					                                      module.abiTypeBuilder(),
					                                      typeTemplateArguments);
					return constantGenerator.getSizeTValue(module.abi().typeInfo().getTypeRequiredAlign(abiType).asBytes() - 1);
				}
				case METHOD_SIZEOF: {
					const auto abiType = this->getABIType(module,
					                                      module.abiTypeBuilder(),
					                                      typeTemplateArguments);
					return constantGenerator.getSizeTValue(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes());
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return methodOwner;
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto destPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                               moveToPtr,
					                                               moveToPosition);
					irEmitter.emitRawStore(methodOwner, destPtr);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_CREATE:
					return zero;
				case METHOD_UNIT:
					return constantGenerator.getPrimitiveInt(primitiveID, 1);
				case METHOD_SETDEAD:
					// Do nothing.
					return constantGenerator.getVoidUndef();
				case METHOD_IMPLICITCASTFROM:
				case METHOD_CASTFROM: {
					const auto argPrimitiveID = methodID.primitiveID();
					const auto operand = args[0].resolve(function);
					const auto selfType = this->getIRType(module,
					                                      typeGenerator,
					                                      typeTemplateArguments);
					if (argPrimitiveID.isFloat()) {
						return builder.CreateFPToSI(operand, selfType);
					} else if (argPrimitiveID.isInteger()) {
						return builder.CreateSExtOrTrunc(operand, selfType);
					} else {
						llvm_unreachable("Unknown signed integer cast source type.");
					}
				}
				case METHOD_IMPLICITCAST:
				case METHOD_CAST: {
					AST::ValueArray valueArray;
					for (const auto& value: typeTemplateArguments) {
						valueArray.push_back(value.copy());
					}
					const auto type = AST::Type::Object(&typeInstance_,
					                                    std::move(valueArray));
					return callCastMethod(function,
					                      methodOwner,
					                      type,
					                      methodID,
					                      functionTemplateArguments.front().typeRefType(),
					                      hintResultValue);
				}
				case METHOD_PLUS:
					return methodOwner;
				case METHOD_MINUS:
					return builder.CreateNeg(methodOwner);
				case METHOD_ISZERO:
					return irEmitter.emitI1ToBool(builder.CreateICmpEQ(methodOwner, zero));
				case METHOD_ISPOSITIVE:
					return irEmitter.emitI1ToBool(builder.CreateICmpSGT(methodOwner, zero));
				case METHOD_ISNEGATIVE:
					return irEmitter.emitI1ToBool(builder.CreateICmpSLT(methodOwner, zero));
				case METHOD_UNSIGNEDVALUE:
					return methodOwner;
				case METHOD_ABS: {
					// Generates: (value < 0) ? -value : value.
					const auto lessThanZero = builder.CreateICmpSLT(methodOwner, zero);
					return builder.CreateSelect(lessThanZero, builder.CreateNeg(methodOwner), methodOwner);
				}
				case METHOD_ADD: {
					const auto operand = args[1].resolveWithoutBind(function);
					if (unsafe) {
						return builder.CreateAdd(methodOwner,
						                         operand,
						                         /*name=*/"",
						                         /*hasNUW=*/false,
						                         /*hasNSW=*/true);
					} else {
						llvm::Value* const binaryArgs[] = { methodOwner, operand };
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::sadd_with_overflow, binaryArgs);
					}
				}
				case METHOD_SUBTRACT: {
					const auto operand = args[1].resolveWithoutBind(function);
					if (unsafe) {
						return builder.CreateSub(methodOwner,
						                         operand,
						                         /*name=*/"",
						                         /*hasNUW=*/false,
						                         /*hasNSW=*/true);
					} else {
						llvm::Value* const binaryArgs[] = { methodOwner, operand };
						return callArithmeticNoOverflowIntrinsic(function, llvm::Intrinsic::ssub_with_overflow, binaryArgs);
					}
				}
				case METHOD_MULTIPLY: {
					const auto operand = args[1].resolveWithoutBind(function);
					if (unsafe) {
						return builder.CreateMul(methodOwner,
						                         operand,
						                         /*name=*/"",
						                         /*hasNUW=*/false,
						                         /*hasNSW=*/true);
					} else {
						const auto checkDivBB = irEmitter.createBasicBlock("");
						const auto trapBB = irEmitter.createBasicBlock("");
						const auto endBB = irEmitter.createBasicBlock("");
						
						const auto mulResult = builder.CreateMul(methodOwner,
						                                         operand);
						
						// Check if methodOwner == 0.
						const auto methodOwnerIsZero = builder.CreateICmpEQ(methodOwner,
						                                                    zero);
						irEmitter.emitCondBranch(methodOwnerIsZero,
						                         endBB, checkDivBB);
						
						// If methodOwner != 0, check (mulResult / methodOwner) == operand.
						irEmitter.selectBasicBlock(checkDivBB);
						const auto divResult = builder.CreateSDiv(mulResult,
						                                          methodOwner);
						const auto divResultIsOperand = builder.CreateICmpEQ(divResult,
						                                                     operand);
						irEmitter.emitCondBranch(divResultIsOperand,
						                         endBB, trapBB);
						
						// (mulResult / methodOwner) != operand -> trap.
						irEmitter.selectBasicBlock(trapBB);
						callTrapIntrinsic(function);
						
						irEmitter.selectBasicBlock(endBB);
						return mulResult;
					}
				}
				case METHOD_DIVIDE: {
					const auto operand = args[1].resolveWithoutBind(function);
					if (!unsafe) {
						// TODO: also check for case of MIN_INT / -1 leading to overflow.
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = irEmitter.createBasicBlock("isZero");
						const auto isNotZeroBB = irEmitter.createBasicBlock("isNotZero");
						irEmitter.emitCondBranch(divisorIsZero, isZeroBB,
						                         isNotZeroBB);
						irEmitter.selectBasicBlock(isZeroBB);
						callTrapIntrinsic(function);
						irEmitter.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateSDiv(methodOwner, operand);
				}
				case METHOD_MODULO: {
					const auto operand = args[1].resolveWithoutBind(function);
					if (!unsafe) {
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = irEmitter.createBasicBlock("isZero");
						const auto isNotZeroBB = irEmitter.createBasicBlock("isNotZero");
						irEmitter.emitCondBranch(divisorIsZero, isZeroBB,
						                         isNotZeroBB);
						irEmitter.selectBasicBlock(isZeroBB);
						callTrapIntrinsic(function);
						irEmitter.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateSRem(methodOwner, operand);
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
					return irEmitter.emitI1ToBool(builder.CreateICmpSLT(methodOwner, operand));
				}
				case METHOD_LESSTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpSLE(methodOwner, operand));
				}
				case METHOD_GREATERTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpSGT(methodOwner, operand));
				}
				case METHOD_GREATERTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpSGE(methodOwner, operand));
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto isLessThan = builder.CreateICmpSLT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpSGT(methodOwner, operand);
					const auto minusOneResult = constantGenerator.getI8(-1);
					const auto zeroResult = constantGenerator.getI8(0);
					const auto plusOneResult = constantGenerator.getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				case METHOD_INCREMENT: {
					// TODO: add safety checks!
					const auto methodOwnerPtr = args[0].resolve(function);
					const auto unit = constantGenerator.getPrimitiveInt(primitiveID, 1);
					const auto incrementedValue = builder.CreateAdd(methodOwner, unit,
					                                                /*name=*/"",
					                                                /*hasNUW=*/false,
					                                                /*hasNSW=*/true);
					irEmitter.emitRawStore(incrementedValue, methodOwnerPtr);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_DECREMENT: {
					// TODO: add safety checks!
					const auto methodOwnerPtr = args[0].resolve(function);
					const auto unit = constantGenerator.getPrimitiveInt(primitiveID, 1);
					const auto decrementedValue = builder.CreateSub(methodOwner, unit,
					                                                /*name=*/"",
					                                                /*hasNUW=*/false,
					                                                /*hasNSW=*/true);
					irEmitter.emitRawStore(decrementedValue, methodOwnerPtr);
					return constantGenerator.getVoidUndef();
				}
				default:
					llvm_unreachable("Unknown primitive method.");
			}
		}
		
	}
	
}

