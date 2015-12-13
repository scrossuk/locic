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
			llvm_abi::Type memberTypes[] = { memberType, memberType };
			return abiTypeBuilder.getStructTy(memberTypes);
		}
		
		llvm::Type* RangePrimitive::getIRType(Module& module,
		                                      const TypeGenerator& typeGenerator,
		                                      llvm::ArrayRef<SEM::Value> templateArguments) const {
			const auto memberType = genType(module, templateArguments.front().typeRefType());
			llvm::Type* const memberTypes[] = { memberType, memberType };
			return typeGenerator.getStructType(memberTypes);
		}
		
		class RangeElementAccess {
		public:
			RangeElementAccess(IREmitter& irEmitter, const SEM::Type* const targetType)
			: irEmitter_(irEmitter), targetType_(targetType),
			sizeValue_(nullptr) { }
			
			llvm::Value* getTargetSize() {
				if (sizeValue_ == nullptr) {
					sizeValue_ = irEmitter_.emitSizeOf(targetType_);
				}
				return sizeValue_;
			}
			
			llvm::Value* getFirstPtr(llvm::Value* const ptr) {
				auto& module = irEmitter_.module();
				
				TypeInfo typeInfo(module);
				if (typeInfo.isSizeAlwaysKnown(targetType_)) {
					// If possible use a GEP rather than a bitcast.
					const auto targetIRType = genType(module, targetType_);
					llvm::Type* const memberTypes[] = { targetIRType, targetIRType };
					const auto structType = irEmitter_.typeGenerator().getStructType(memberTypes);
					return irEmitter_.emitConstInBoundsGEP2_32(structType,
					                                           ptr, 0, 0);
				} else {
					return ptr;
				}
			}
			
			llvm::Value* getSecondPtr(llvm::Value* const ptr) {
				auto& module = irEmitter_.module();
				
				TypeInfo typeInfo(module);
				if (typeInfo.isSizeAlwaysKnown(targetType_)) {
					const auto targetIRType = genType(module, targetType_);
					llvm::Type* const memberTypes[] = { targetIRType, targetIRType };
					const auto structType = irEmitter_.typeGenerator().getStructType(memberTypes);
					return irEmitter_.emitConstInBoundsGEP2_32(structType,
					                                           ptr, 0, 1);
				} else {
					return irEmitter_.emitInBoundsGEP(irEmitter_.typeGenerator().getI8Type(),
					                                  ptr, getTargetSize());
				}
			}
			
		private:
			IREmitter& irEmitter_;
			const SEM::Type* targetType_;
			llvm::Value* sizeValue_;
			
		};
		
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
			
			RangeElementAccess elementAccess(irEmitter, targetType);
			
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
					
					const auto destPtrFirst = elementAccess.getFirstPtr(result);
					const auto firstArgumentValue = args[0].resolve(function, destPtrFirst); 
					irEmitter.emitMoveStore(firstArgumentValue,
					                        destPtrFirst, targetType);
					
					const auto destPtrSecond = elementAccess.getSecondPtr(result);
					const auto secondArgumentValue = args[1].resolve(function, destPtrSecond);
					irEmitter.emitMoveStore(secondArgumentValue,
					                        destPtrSecond, targetType);
					
					return irEmitter.emitMoveLoad(result, type);
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY: {
					auto methodOwner = args[0].resolve(function);
					
					const auto result = irEmitter.emitReturnAlloca(type);
					
					// Copy first element of range pair.
					const auto copySourcePtrFirst = elementAccess.getFirstPtr(methodOwner);
					const auto copyDestPtrFirst = elementAccess.getFirstPtr(result);
					const auto copyResultFirst =
					    irEmitter.emitCopyCall(methodID,
					                           copySourcePtrFirst,
					                           targetType,
					                           copyDestPtrFirst);
					irEmitter.emitMoveStore(copyResultFirst,
					                        copyDestPtrFirst,
					                        targetType);
					
					// Copy second element of range pair.
					const auto copySourcePtrSecond = elementAccess.getSecondPtr(methodOwner);
					const auto copyDestPtrSecond = elementAccess.getSecondPtr(result);
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
					
					// Move first element of range pair.
					const auto pairFirstPtr = elementAccess.getFirstPtr(methodOwner);
					const auto moveToPositionFirst = moveToPosition;
					irEmitter.emitMoveCall(pairFirstPtr, moveToPtr,
					                       moveToPositionFirst, targetType);
					
					// Move second element of range pair.
					const auto pairSecondPtr = elementAccess.getSecondPtr(methodOwner);
					const auto moveToPositionSecond = irEmitter.builder().CreateAdd(moveToPosition,
					                                                                elementAccess.getTargetSize());
					irEmitter.emitMoveCall(pairSecondPtr, moveToPtr,
					                       moveToPositionSecond, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_DESTROY: {
					auto methodOwner = args[0].resolve(function);
					irEmitter.emitDestructorCall(elementAccess.getSecondPtr(methodOwner),
					                             targetType);
					irEmitter.emitDestructorCall(elementAccess.getFirstPtr(methodOwner),
					                             targetType);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_FRONT: {
					return elementAccess.getFirstPtr(args[0].resolve(function));
				}
				case METHOD_BACK: {
					return elementAccess.getSecondPtr(args[0].resolve(function));
				}
				case METHOD_SKIPFRONT: {
					const auto pairFirstPtr = elementAccess.getFirstPtr(args[0].resolve(function));
					if (typeInstance_.primitiveID() == PrimitiveCount ||
					    typeInstance_.primitiveID() == PrimitiveRange ||
					    typeInstance_.primitiveID() == PrimitiveRangeIncl) {
						irEmitter.emitNoArgNoReturnCall(METHOD_INCREMENT,
						                                pairFirstPtr,
						                                targetType);
					} else {
						irEmitter.emitNoArgNoReturnCall(METHOD_DECREMENT,
						                                pairFirstPtr,
						                                targetType);
					}
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_SKIPBACK: {
					const auto pairSecondPtr = elementAccess.getSecondPtr(args[0].resolve(function));
					if (typeInstance_.primitiveID() == PrimitiveCount ||
					    typeInstance_.primitiveID() == PrimitiveRange ||
					    typeInstance_.primitiveID() == PrimitiveRangeIncl) {
						irEmitter.emitNoArgNoReturnCall(METHOD_DECREMENT,
						                                pairSecondPtr,
						                                targetType);
					} else {
						irEmitter.emitNoArgNoReturnCall(METHOD_INCREMENT,
						                                pairSecondPtr,
						                                targetType);
					}
					return ConstantGenerator(module).getVoidUndef();
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

