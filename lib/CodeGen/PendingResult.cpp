#include <locic/AST/Type.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/PendingResult.hpp>
#include <locic/CodeGen/TypeInfo.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ValuePendingResult::ValuePendingResult(llvm::Value* const value,
		                                       const AST::Type* const type)
		: value_(value),
		  type_(type) { }
		
		llvm::Value*
		ValuePendingResult::generateValue(Function& function,
		                                  llvm::Value* const resultPtr) const {
			if (resultPtr != nullptr) {
				IREmitter irEmitter(function);
				irEmitter.emitMoveStore(value_, resultPtr, type_);
				return resultPtr;
			} else {
				return value_;
			}
		}
		
		llvm::Value* ValuePendingResult::generateLoadedValue(Function& function) const {
			if (type_ == nullptr) {
				llvm_unreachable("This pending result wasn't supposed to be loaded.");
			}
			assert(type_->isRef());
			assert(value_->getType()->isPointerTy());
			
			const auto refTargetType = type_->templateArguments().front().typeRefType();
			
			IREmitter irEmitter(function);
			return irEmitter.emitLoad(value_, refTargetType);
		}
		
		RefPendingResult::RefPendingResult(llvm::Value* const refValue,
		                                   const AST::Type* const refTargetType)
		: refValue_(refValue),
		refTargetType_(refTargetType) { }
		
		llvm::Value* RefPendingResult::generateValue(Function& /*function*/, llvm::Value* /*resultPtr*/) const {
			// resultPtr can be ignored because references are handled by value.
			return refValue_;
		}
		
		llvm::Value* RefPendingResult::generateLoadedValue(Function& function) const {
			return IREmitter(function).emitLoad(refValue_, refTargetType_);
		}
		
		ValueToRefPendingResult::ValueToRefPendingResult(llvm::Value* const value,
		                                                 const AST::Type* const refTargetType)
		: value_(value), refTargetType_(refTargetType) { }
		
		llvm::Value* ValueToRefPendingResult::generateValue(Function& function,
		                                                    llvm::Value* const /*resultPtr*/) const {
			// resultPtr can be ignored because references are handled by value.
			return IREmitter(function).emitBind(value_, refTargetType_);
		}
		
		llvm::Value* ValueToRefPendingResult::generateLoadedValue(Function& /*function*/) const {
			return value_;
		}
		
		PendingResult::PendingResult(const PendingResultBase& base)
		: base_(&base), cacheLastResultPtr_(nullptr),
		 cacheLastResolvedValue_(nullptr),
		 cacheLastResolvedWithoutBindValue_(nullptr) { }
		
		llvm::Value* PendingResult::resolve(Function& function, llvm::Value* const resultPtr) {
			if (cacheLastResolvedValue_ != nullptr &&
			    resultPtr == cacheLastResultPtr_) {
				// Return cached result.
				return cacheLastResolvedValue_;
			}
			
			const auto result = base_->generateValue(function, resultPtr);
			assert(result != nullptr);
			
			cacheLastResultPtr_ = resultPtr;
			cacheLastResolvedValue_ = result;
			
			return result;
		}
		
		llvm::Value* PendingResult::resolveWithoutBind(Function& function) {
			if (cacheLastResolvedWithoutBindValue_ != nullptr) {
				return cacheLastResolvedWithoutBindValue_;
			}
			
			const auto result = base_->generateLoadedValue(function);
			assert(result != nullptr);
			
			cacheLastResolvedWithoutBindValue_ = result;
			
			return result;
		}
		
	}
	
}
