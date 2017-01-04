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
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/PtrPrimitive.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		PtrPrimitive::PtrPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) {
			(void) typeInstance_;
		}
		
		bool PtrPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                     llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool PtrPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                           llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool PtrPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool PtrPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type PtrPrimitive::getABIType(Module& /*module*/,
		                                        const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                        llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return llvm_abi::PointerTy;
		}
		
		llvm::Type* PtrPrimitive::getIRType(Module& /*module*/,
		                                    const TypeGenerator& typeGenerator,
		                                    llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return typeGenerator.getPtrType();
		}
		
		llvm::Value* PtrPrimitive::emitMethod(IREmitter& irEmitter,
		                                      const MethodID methodID,
		                                      llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                      llvm::ArrayRef<AST::Value> /*functionTemplateArguments*/,
		                                      PendingResultArray args,
		                                      llvm::Value* /*hintResultValue*/) const {
			auto& builder = irEmitter.builder();
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto irType = irEmitter.typeGenerator().getPtrType();
			
			const auto targetType = typeTemplateArguments.front().typeRefType();
			const auto methodOwnerPointer = methodID.isConstructor() ? nullptr : args[0].resolve(function);
			const auto methodOwner = methodOwnerPointer != nullptr ?
			                         irEmitter.emitRawLoad(methodOwnerPointer, irType) :
			                         nullptr;
			
			switch (methodID) {
				case METHOD_NULL: {
					return ConstantGenerator(module).getNull(irType);
				}
				case METHOD_ALIGNMASK: {
					const auto abiType = this->getABIType(module,
					                                      module.abiTypeBuilder(),
					                                      typeTemplateArguments);
					return ConstantGenerator(module).getSizeTValue(module.abi().typeInfo().getTypeRequiredAlign(abiType).asBytes() - 1);
				}
				case METHOD_SIZEOF: {
					const auto abiType = this->getABIType(module,
					                                      module.abiTypeBuilder(),
					                                      typeTemplateArguments);
					return ConstantGenerator(module).getSizeTValue(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes());
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY:
				case METHOD_MOVE:
				case METHOD_DEREF:
					return methodOwner;
				case METHOD_SETDEAD: {
					// Do nothing.
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_INCREMENT: {
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeKnownInThisModule(targetType)) {
						const auto one = ConstantGenerator(module).getI32(1);
						const auto newPointer = irEmitter.emitInBoundsGEP(genType(module, targetType),
						                                                  methodOwner,
						                                                  one);
						irEmitter.emitRawStore(newPointer, methodOwnerPointer);
					} else {
						const auto targetSize = genSizeOf(function, targetType);
						const auto newPointer = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
						                                                  methodOwner,
						                                                  targetSize);
						irEmitter.emitRawStore(newPointer, methodOwnerPointer);
					}
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_DECREMENT: {
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeKnownInThisModule(targetType)) {
						const auto minusOne = ConstantGenerator(module).getI32(-1);
						const auto newPointer = irEmitter.emitInBoundsGEP(genType(module, targetType),
						                                                  methodOwner,
						                                                  minusOne);
						irEmitter.emitRawStore(newPointer, methodOwnerPointer);
					} else {
						const auto targetSize = genSizeOf(function, targetType);
						const auto minusTargetSize = builder.CreateNeg(targetSize);
						const auto newPointer = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
						                                                  methodOwner,
						                                                  minusTargetSize);
						irEmitter.emitRawStore(newPointer, methodOwnerPointer);
					}
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ADD: {
					const auto operand = args[1].resolveWithoutBind(function);
					
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeKnownInThisModule(targetType)) {
						return irEmitter.emitInBoundsGEP(genType(module, targetType),
						                                 methodOwner,
						                                 operand);
					} else {
						const auto targetSize = genSizeOf(function, targetType);
						const auto adjustedOffset = builder.CreateMul(operand, targetSize);
						return irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
						                                 methodOwner,
						                                 adjustedOffset);
					}
				}
				case METHOD_SUBTRACT: {
					// TODO: should be intptr_t!
					const auto ptrDiffTType = getBasicPrimitiveType(module, PrimitivePtrDiff);
					const auto operand = args[1].resolveWithoutBind(function);
					
					const auto firstPtrInt = builder.CreatePtrToInt(methodOwner, ptrDiffTType);
					const auto secondPtrInt = builder.CreatePtrToInt(operand, ptrDiffTType);
					
					return builder.CreateSub(firstPtrInt, secondPtrInt);
				}
				case METHOD_INDEX: {
					const auto sizeTType = getBasicPrimitiveType(module, PrimitiveSize);
					const auto operand = args[1].resolve(function);
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeKnownInThisModule(targetType)) {
						return irEmitter.emitInBoundsGEP(genType(module, targetType),
						                                 methodOwner,
						                                 operand);
					} else {
						const auto targetSize = genSizeOf(function, targetType);
						const auto offset = builder.CreateIntCast(operand, sizeTType, true);
						const auto adjustedOffset = builder.CreateMul(offset, targetSize);
						return irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
						                                 methodOwner,
						                                 adjustedOffset);
					}
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
				default:
					llvm_unreachable("Unknown ptr primitive method.");
			}
		}
		
	}
	
}

