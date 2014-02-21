#include <locic/SEM.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		UnwindAction UnwindAction::Destroy(SEM::Type* type, llvm::Value* value) {
			UnwindAction action(UnwindAction::DESTRUCTOR);
			action.destructor_.type = type;
			action.destructor_.value = value;
			return action;
		}
		
		UnwindAction UnwindAction::CatchException(llvm::BasicBlock* catchBlock) {
			UnwindAction action(UnwindAction::CATCH);
			action.catchBlock_ = catchBlock;
			return action;
		}
		
		UnwindAction UnwindAction::ScopeMarker() {
			return UnwindAction(UnwindAction::SCOPEMARKER);
		}
		
		UnwindAction::Kind UnwindAction::kind() const {
			return kind_;
		}
		
		bool UnwindAction::isDestructor() const {
			return kind() == UnwindAction::DESTRUCTOR;
		}
		
		bool UnwindAction::isCatch() const {
			return kind() == UnwindAction::CATCH;
		}
		
		bool UnwindAction::isScopeMarker() const {
			return kind() == UnwindAction::SCOPEMARKER;
		}
		
		SEM::Type* UnwindAction::destroyType() const {
			assert(isDestructor());
			return destructor_.type;
		}
		
		llvm::Value* UnwindAction::destroyValue() const {
			assert(isDestructor());
			return destructor_.value;
		}
		
		llvm::BasicBlock* UnwindAction::catchBlock() const {
			assert(isCatch());
			return catchBlock_;
		}
		
	}
	
}

