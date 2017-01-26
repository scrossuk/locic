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
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
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
		
		RangePrimitive::RangePrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool RangePrimitive::isSizeAlwaysKnown(const TypeInfo& typeInfo,
		                                       llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.isSizeAlwaysKnown(templateArguments.front().typeRefType());
		}
		
		bool RangePrimitive::isSizeKnownInThisModule(const TypeInfo& typeInfo,
		                                           llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.isSizeKnownInThisModule(templateArguments.front().typeRefType());
		}
		
		bool RangePrimitive::hasCustomDestructor(const TypeInfo& typeInfo,
		                                         llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.hasCustomDestructor(templateArguments.front().typeRefType());
		}
		
		bool RangePrimitive::hasCustomMove(const TypeInfo& typeInfo,
		                                   llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.hasCustomMove(templateArguments.front().typeRefType());
		}
		
		llvm_abi::Type RangePrimitive::getABIType(Module& module,
		                                          const llvm_abi::TypeBuilder& abiTypeBuilder,
		                                          llvm::ArrayRef<AST::Value> templateArguments) const {
			const auto memberType = genABIType(module, templateArguments.front().typeRefType());
			llvm_abi::Type memberTypes[] = { memberType, memberType };
			return abiTypeBuilder.getStructTy(memberTypes);
		}
		
		class RangeElementAccess {
		public:
			RangeElementAccess(IREmitter& irEmitter, const AST::Type* const targetType)
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
					const auto targetABIType = genABIType(module, targetType_);
					const llvm_abi::Type memberTypes[] = { targetABIType, targetABIType };
					const auto structType = module.abiTypeBuilder().getStructTy(memberTypes);
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
					const auto targetABIType = genABIType(module, targetType_);
					const llvm_abi::Type memberTypes[] = { targetABIType, targetABIType };
					const auto structType = module.abiTypeBuilder().getStructTy(memberTypes);
					return irEmitter_.emitConstInBoundsGEP2_32(structType,
					                                           ptr, 0, 1);
				} else {
					return irEmitter_.emitInBoundsGEP(llvm_abi::Int8Ty,
					                                  ptr, getTargetSize());
				}
			}
			
		private:
			IREmitter& irEmitter_;
			const AST::Type* targetType_;
			llvm::Value* sizeValue_;
			
		};
		
		llvm::Value* RangePrimitive::emitMethod(IREmitter& irEmitter,
		                                        const MethodID methodID,
		                                        llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                        llvm::ArrayRef<AST::Value> /*functionTemplateArguments*/,
		                                        PendingResultArray args,
		                                        llvm::Value* const resultPtr) const {
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			AST::ValueArray valueArray;
			for (const auto& value: typeTemplateArguments) {
				valueArray.push_back(value.copy());
			}
			const auto type = AST::Type::Object(&typeInstance_,
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
					const auto result = irEmitter.emitAlloca(type, resultPtr);
					
					const auto destPtrFirst = elementAccess.getFirstPtr(result);
					const auto firstArgumentValue = args[0].resolve(function, destPtrFirst);
					irEmitter.emitStore(firstArgumentValue, destPtrFirst, targetType);
					
					const auto destPtrSecond = elementAccess.getSecondPtr(result);
					const auto secondArgumentValue = args[1].resolve(function, destPtrSecond);
					irEmitter.emitStore(secondArgumentValue, destPtrSecond, targetType);
					
					return irEmitter.emitLoad(result, type);
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY: {
					auto methodOwner = args[0].resolve(function);
					
					const auto result = irEmitter.emitAlloca(type, resultPtr);
					
					// Copy first element of range pair.
					const auto copySourcePtrFirst = elementAccess.getFirstPtr(methodOwner);
					const auto copyDestPtrFirst = elementAccess.getFirstPtr(result);
					const auto copyResultFirst =
					    irEmitter.emitCopyCall(methodID,
					                           copySourcePtrFirst,
					                           targetType,
					                           copyDestPtrFirst);
					irEmitter.emitStore(copyResultFirst,
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
					irEmitter.emitStore(copyResultSecond,
					                    copyDestPtrSecond,
					                    targetType);
					
					return irEmitter.emitLoad(result, type);
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
				case METHOD_MOVE: {
					const auto destPtr = irEmitter.emitAlloca(type, resultPtr);
					const auto sourcePtr = args[0].resolve(function);
					
					// Move first element of range pair.
					const auto sourceFirstPtr = elementAccess.getFirstPtr(sourcePtr);
					const auto destFirstPtr = elementAccess.getFirstPtr(destPtr);
					irEmitter.emitMove(sourceFirstPtr, destFirstPtr, targetType);
					
					// Move second element of range pair.
					const auto sourceSecondPtr = elementAccess.getSecondPtr(sourcePtr);
					const auto destSecondPtr = elementAccess.getSecondPtr(destPtr);
					irEmitter.emitMove(sourceSecondPtr, destSecondPtr, targetType);
					
					return irEmitter.emitLoad(destPtr, type);
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
					if (typeInstance_.primitiveID() == PrimitiveRange ||
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
					if (typeInstance_.primitiveID() == PrimitiveRange ||
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
					auto methodOwner = args[0].resolve(function);
					const auto pairFirstPtr = elementAccess.getFirstPtr(methodOwner);
					const auto pairSecondPtr = elementAccess.getSecondPtr(methodOwner);
					
					RefPendingResult pairFirstPendingResult(pairFirstPtr, targetType);
					RefPendingResult pairSecondPendingResult(pairSecondPtr, targetType);
					
					if (typeInstance_.primitiveID() == PrimitiveRange) {
						const auto result = irEmitter.emitComparisonCall(METHOD_LESSTHAN,
						                                                 pairFirstPendingResult,
						                                                 pairSecondPendingResult,
						                                                 targetType);
						const auto i1Value = irEmitter.emitBoolToI1(result);
						const auto i1NotValue = irEmitter.builder().CreateNot(i1Value);
						return irEmitter.emitI1ToBool(i1NotValue);
					} else if (typeInstance_.primitiveID() == PrimitiveRangeIncl) {
						return irEmitter.emitComparisonCall(METHOD_LESSTHAN,
						                                    pairSecondPendingResult,
						                                    pairFirstPendingResult,
						                                    targetType);
					} else if (typeInstance_.primitiveID() == PrimitiveReverseRange) {
						const auto result = irEmitter.emitComparisonCall(METHOD_LESSTHAN,
						                                                 pairSecondPendingResult,
						                                                 pairFirstPendingResult,
						                                                 targetType);
						const auto i1Value = irEmitter.emitBoolToI1(result);
						const auto i1NotValue = irEmitter.builder().CreateNot(i1Value);
						return irEmitter.emitI1ToBool(i1NotValue);
					} else {
						return irEmitter.emitComparisonCall(METHOD_LESSTHAN,
						                                    pairFirstPendingResult,
						                                    pairSecondPendingResult,
						                                    targetType);
					}
				}
				default:
					llvm_unreachable("Unknown range primitive method.");
			}
		}
		
	}
	
}

