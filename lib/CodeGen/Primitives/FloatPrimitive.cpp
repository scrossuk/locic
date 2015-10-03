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
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/FloatPrimitive.hpp>
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
		
		FloatPrimitive::FloatPrimitive(const SEM::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool FloatPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                       llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool FloatPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                             llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool FloatPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool FloatPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type* FloatPrimitive::getABIType(Module& /*module*/,
		                                           llvm_abi::Context& abiContext,
		                                           llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			switch (typeInstance_.primitiveID()) {
				case PrimitiveFloat:
					return llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::Float);
				case PrimitiveDouble:
					return llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::Double);
				case PrimitiveLongDouble:
					return llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::LongDouble);
				default:
					llvm_unreachable("Invalid float primitive ID.");
			}
			
		}
		
		llvm::Type* FloatPrimitive::getIRType(Module& /*module*/,
		                                      const TypeGenerator& typeGenerator,
		                                      llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			switch (typeInstance_.primitiveID()) {
				case PrimitiveFloat:
					return typeGenerator.getFloatType();
				case PrimitiveDouble:
					return typeGenerator.getDoubleType();
				case PrimitiveLongDouble:
					return typeGenerator.getLongDoubleType();
				default:
					llvm_unreachable("Invalid float primitive ID.");
			}
		}
		
		llvm::Value* FloatPrimitive::emitMethod(IREmitter& irEmitter,
		                                        const MethodID methodID,
		                                        llvm::ArrayRef<SEM::Value> typeTemplateArguments,
		                                        llvm::ArrayRef<SEM::Value> functionTemplateArguments,
		                                        PendingResultArray args) const {
			auto& builder = irEmitter.builder();
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto& constantGenerator = irEmitter.constantGenerator();
			
			const auto primitiveID = typeInstance_.primitiveID();
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function);
			
			switch (methodID) {
				case METHOD_ALIGNMASK: {
					const auto abiType = this->getABIType(module,
					                                      module.abiContext(),
					                                      typeTemplateArguments);
					return constantGenerator.getSizeTValue(module.abi().typeAlign(abiType) - 1);
				}
				case METHOD_SIZEOF: {
					const auto abiType = this->getABIType(module,
					                                      module.abiContext(),
					                                      typeTemplateArguments);
					return constantGenerator.getSizeTValue(module.abi().typeSize(abiType));
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return methodOwner;
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto irType = this->getIRType(module,
					                                    irEmitter.typeGenerator(),
					                                    typeTemplateArguments);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, irType->getPointerTo());
					builder.CreateStore(methodOwner, castedDestPtr);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_CREATE:
					return constantGenerator.getPrimitiveFloat(primitiveID, 0.0);
				case METHOD_SETDEAD:
					// Do nothing.
					return constantGenerator.getVoidUndef();
				case METHOD_ISLIVE:
					return constantGenerator.getI1(true);
				case METHOD_IMPLICITCASTFROM:
				case METHOD_CASTFROM: {
					const auto argPrimitiveID = methodID.primitiveID();
					const auto operand = args[0].resolve(function);
					const auto selfType = this->getIRType(module,
					                                      irEmitter.typeGenerator(),
					                                      typeTemplateArguments);
					if (argPrimitiveID.isFloat()) {
						return builder.CreateFPCast(operand, selfType);
					} else if (argPrimitiveID.isUnsignedInteger()) {
						return builder.CreateUIToFP(operand, selfType);
					} else if (argPrimitiveID.isSignedInteger()) {
						return builder.CreateSIToFP(operand, selfType);
					} else {
						llvm_unreachable("Unknown float cast source type.");
					}
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
				case METHOD_PLUS:
					return methodOwner;
				case METHOD_MINUS:
					return builder.CreateFNeg(methodOwner);
				case METHOD_ISZERO: {
					const auto zero = constantGenerator.getPrimitiveFloat(primitiveID, 0.0);
					return builder.CreateFCmpOEQ(methodOwner, zero);
				}
				case METHOD_ISPOSITIVE: {
					const auto zero = constantGenerator.getPrimitiveFloat(primitiveID, 0.0);
					return builder.CreateFCmpOGT(methodOwner, zero);
				}
				case METHOD_ISNEGATIVE: {
					const auto zero = constantGenerator.getPrimitiveFloat(primitiveID, 0.0);
					return builder.CreateFCmpOLT(methodOwner, zero);
				}
				case METHOD_ABS: {
					// Generates: (value < 0) ? -value : value.
					const auto zero = constantGenerator.getPrimitiveFloat(primitiveID, 0.0);
					const auto lessThanZero = builder.CreateFCmpOLT(methodOwner, zero);
					return builder.CreateSelect(lessThanZero, builder.CreateFNeg(methodOwner), methodOwner);
				}
				case METHOD_SQRT: {
					llvm::Type* const intrinsicTypes[] = { methodOwner->getType() };
					const auto sqrtIntrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::sqrt, intrinsicTypes);
					llvm::Value* const sqrtArgs[] = { methodOwner };
					return builder.CreateCall(sqrtIntrinsic, sqrtArgs);
				}
				case METHOD_ADD: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFAdd(methodOwner, operand);
				}
				case METHOD_SUBTRACT: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFSub(methodOwner, operand);
				}
				case METHOD_MULTIPLY: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFMul(methodOwner, operand);
				}
				case METHOD_DIVIDE: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFDiv(methodOwner, operand);
				}
				case METHOD_MODULO: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFRem(methodOwner, operand);
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOEQ(methodOwner, operand);
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpONE(methodOwner, operand);
				}
				case METHOD_LESSTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOLT(methodOwner, operand);
				}
				case METHOD_LESSTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOLE(methodOwner, operand);
				}
				case METHOD_GREATERTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOGT(methodOwner, operand);
				}
				case METHOD_GREATERTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOGE(methodOwner, operand);
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto isLessThan = builder.CreateFCmpOLT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateFCmpOGT(methodOwner, operand);
					const auto minusOne = constantGenerator.getI8(-1);
					const auto zero = constantGenerator.getI8(0);
					const auto plusOne = constantGenerator.getI8(1);
					return builder.CreateSelect(isLessThan, minusOne,
							builder.CreateSelect(isGreaterThan, plusOne, zero));
				}
				default:
					llvm_unreachable("Unknown primitive method.");
			}
		}
		
	}
	
}
