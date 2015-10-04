#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/PendingResult.hpp>
#include <locic/SEM/Type.hpp>

namespace locic {
	
	namespace CodeGen {
		
		RefPendingResult::RefPendingResult(llvm::Value* const value,
		                                       const SEM::Type* const refTargetType)
		: value_(value),
		  refTargetType_(refTargetType) { }
		
		llvm::Value* RefPendingResult::generateValue(Function& /*function*/, llvm::Value* /*hintResultValue*/) const {
			return value_;
		}
		
		llvm::Value* RefPendingResult::generateLoadedValue(Function& function) const {
			return genMoveLoad(function, value_, refTargetType_);
		}
		
		ValuePendingResult::ValuePendingResult(llvm::Value* const value,
		                                       const SEM::Type* const type)
		: value_(value),
		  type_(type) { }
		
		llvm::Value* ValuePendingResult::generateValue(Function& /*function*/, llvm::Value* /*hintResultValue*/) const {
			return value_;
		}
		
		llvm::Value* ValuePendingResult::generateLoadedValue(Function& function) const {
			if (type_ == nullptr) {
				llvm_unreachable("This pending result wasn't supposed to be loaded.");
			}
			assert(type_->isBuiltInReference());
			assert(value_->getType()->isPointerTy());
			
			const auto refTargetType = type_->templateArguments().front().typeRefType();
			const auto loadType = genType(function.module(),
			                              refTargetType);
			
			IREmitter irEmitter(function);
			return irEmitter.emitRawLoad(value_, loadType);
		}
		
		PendingResult::PendingResult(const PendingResultBase& base)
		: base_(&base),
		 cacheLastHintResultValue_(nullptr),
		 cacheLastResolvedValue_(nullptr),
		 cacheLastResolvedWithoutBindValue_(nullptr) { }
		
		llvm::Value* PendingResult::resolve(Function& function, llvm::Value* const hintResultValue) {
			if (cacheLastResolvedValue_ != nullptr &&
			    hintResultValue == cacheLastHintResultValue_) {
				// Return cached result.
				return cacheLastResolvedValue_;
			}
			
			const auto result = base_->generateValue(function, hintResultValue);
			assert(result != nullptr);
			
			cacheLastHintResultValue_ = hintResultValue;
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
