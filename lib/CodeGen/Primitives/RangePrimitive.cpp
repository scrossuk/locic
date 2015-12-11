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
#include <locic/CodeGen/Primitives/RangePrimitive.hpp>
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
		
		RangePrimitive::RangePrimitive(const SEM::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool RangePrimitive::isSizeAlwaysKnown(const TypeInfo& typeInfo,
		                                       llvm::ArrayRef<SEM::Value> templateArguments) const {
			return typeInfo.isSizeAlwaysKnown(templateArguments.front().typeRefType());
		}
		
		bool RangePrimitive::isSizeKnownInThisModule(const TypeInfo& typeInfo,
		                                           llvm::ArrayRef<SEM::Value> templateArguments) const {
			return typeInfo.isSizeKnownInThisModule(templateArguments.front().typeRefType());
		}
		
		bool RangePrimitive::hasCustomDestructor(const TypeInfo& typeInfo,
		                                         llvm::ArrayRef<SEM::Value> templateArguments) const {
			return typeInfo.hasCustomDestructor(templateArguments.front().typeRefType());
		}
		
		bool RangePrimitive::hasCustomMove(const TypeInfo& typeInfo,
		                                   llvm::ArrayRef<SEM::Value> templateArguments) const {
			return typeInfo.hasCustomMove(templateArguments.front().typeRefType());
		}
		
		llvm_abi::Type RangePrimitive::getABIType(Module& module,
		                                          const llvm_abi::TypeBuilder& abiTypeBuilder,
		                                          llvm::ArrayRef<SEM::Value> templateArguments) const {
			const auto memberType = genABIType(module, templateArguments.front().typeRefType());
			return abiTypeBuilder.getStructTy({ memberType, memberType });
		}
		
		llvm::Type* RangePrimitive::getIRType(Module& module,
		                                      const TypeGenerator& typeGenerator,
		                                      llvm::ArrayRef<SEM::Value> templateArguments) const {
			const auto memberType = genType(module, templateArguments.front().typeRefType());
			return typeGenerator.getStructType({ memberType, memberType });
		}
		
		llvm::Value* RangePrimitive::emitMethod(IREmitter& irEmitter,
		                                      const MethodID methodID,
		                                      llvm::ArrayRef<SEM::Value> typeTemplateArguments,
		                                      llvm::ArrayRef<SEM::Value> /*functionTemplateArguments*/,
		                                      PendingResultArray args) const {
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			SEM::ValueArray valueArray;
			for (const auto& value: typeTemplateArguments) {
				valueArray.push_back(value.copy());
			}
			const auto type = SEM::Type::Object(&typeInstance_,
			                                    std::move(valueArray));
			
			const auto targetType = typeTemplateArguments.front().typeRefType();
			
			switch (methodID) {
				case METHOD_ALIGNMASK: {
					return irEmitter.emitAlignMask(targetType);
				}
				case METHOD_SIZEOF: {
					return irEmitter.emitSizeOf(targetType);
				}
				case METHOD_CREATE: {
					if (typeInstance_.primitiveID() == PrimitiveCount ||
					    typeInstance_.primitiveID() == PrimitiveCountIncl) {
						// TODO: call zero() method to get start of range.
						llvm_unreachable("TODO");
					}
					
					const auto result = irEmitter.emitReturnAlloca(type);
					const auto targetSize = irEmitter.emitSizeOf(targetType);
					
					const auto destPtrFirst = result;
					const auto firstArgumentValue = args[0].resolve(function, destPtrFirst); 
					irEmitter.emitMoveStore(firstArgumentValue,
					                        destPtrFirst, targetType);
					
					const auto destPtrSecond = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                                     result,
					                                                     targetSize);
					const auto secondArgumentValue = args[1].resolve(function, destPtrSecond);
					irEmitter.emitMoveStore(secondArgumentValue,
					                        destPtrSecond, targetType);
					
					return irEmitter.emitMoveLoad(result, type);
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY: {
					auto methodOwner = args[0].resolve(function);
					const auto result = irEmitter.emitReturnAlloca(type);
					const auto targetSize = irEmitter.emitSizeOf(targetType);
					
					// Copy first element of range pair.
					const auto copySourcePtrFirst = methodOwner;
					const auto copyDestPtrFirst = result;
					const auto copyResultFirst =
					    irEmitter.emitCopyCall(methodID,
					                           copySourcePtrFirst,
					                           targetType,
					                           copyDestPtrFirst);
					irEmitter.emitMoveStore(copyResultFirst,
					                        copyDestPtrFirst,
					                        targetType);
					
					// Copy second element of range pair.
					const auto copySourcePtrSecond = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                                           methodOwner,
					                                                           targetSize);
					const auto copyDestPtrSecond = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                                         result,
					                                                         targetSize);
					const auto copyResultSecond =
					    irEmitter.emitCopyCall(methodID,
					                           copySourcePtrSecond,
					                           targetType,
					                           copyDestPtrSecond);
					irEmitter.emitMoveStore(copyResultSecond,
					                        copyDestPtrSecond,
					                        targetType);
					
					return irEmitter.emitMoveLoad(result, type);
				}
				case METHOD_ISLIVE: {
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getBool(true);
				}
				case METHOD_SETDEAD: {
					// Do nothing.
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					auto methodOwner = args[0].resolve(function);
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					const auto targetSize = irEmitter.emitSizeOf(targetType);
					
					// Move first element of range pair.
					const auto pairFirstPtr = methodOwner;
					const auto moveToPositionFirst = moveToPosition;
					irEmitter.emitMoveCall(pairFirstPtr, moveToPtr,
					                       moveToPositionFirst, targetType);
					
					// Move second element of range pair.
					const auto pairSecondPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                                     methodOwner,
					                                                     targetSize);
					const auto moveToPositionSecond = irEmitter.builder().CreateAdd(moveToPosition,
					                                                                targetSize);
					irEmitter.emitMoveCall(pairSecondPtr, moveToPtr,
					                       moveToPositionSecond, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_DESTROY: {
					auto methodOwner = args[0].resolve(function);
					const auto targetSize = irEmitter.emitSizeOf(targetType);
					
					// Destroy first element of range pair.
					const auto pairFirstPtr = methodOwner;
					irEmitter.emitDestructorCall(pairFirstPtr, targetType);
					
					// Destroy second element of range pair.
					const auto pairSecondPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                                     methodOwner,
					                                                     targetSize);
					irEmitter.emitDestructorCall(pairSecondPtr, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_FRONT: {
					return args[0].resolve(function);
				}
				case METHOD_BACK: {
					auto methodOwner = args[0].resolve(function);
					const auto targetSize = irEmitter.emitSizeOf(targetType);
					const auto pairSecondPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                                     methodOwner,
					                                                     targetSize);
					return pairSecondPtr;
				}
				case METHOD_SKIPFRONT: {
					// TODO: call increment() method on first element of range pair.
					llvm_unreachable("TODO");
				}
				case METHOD_SKIPBACK: {
					// TODO: call decrement() method on second element of range pair.
					llvm_unreachable("TODO");
				}
				case METHOD_EMPTY: {
					// TODO: compare range pair elements.
					llvm_unreachable("TODO");
				}
				default:
					llvm_unreachable("Unknown range primitive method.");
			}
		}
		
	}
	
}

