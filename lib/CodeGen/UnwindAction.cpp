#include <locic/SEM.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		UnwindAction UnwindAction::Destroy(SEM::Type* type, llvm::Value* value) {
			UnwindAction action(UnwindAction::DESTRUCTOR);
			action.actions_.destroyAction.type = type;
			action.actions_.destroyAction.value = value;
			return action;
		}
		
		UnwindAction UnwindAction::CatchException(llvm::BasicBlock* catchBlock, llvm::Constant* catchTypeInfo) {
			UnwindAction action(UnwindAction::CATCH);
			action.actions_.catchAction.block = catchBlock;
			action.actions_.catchAction.typeInfo = catchTypeInfo;
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
			return actions_.destroyAction.type;
		}
		
		llvm::Value* UnwindAction::destroyValue() const {
			assert(isDestructor());
			return actions_.destroyAction.value;
		}
		
		llvm::BasicBlock* UnwindAction::catchBlock() const {
			assert(isCatch());
			return actions_.catchAction.block;
		}
		
		llvm::Constant* UnwindAction::catchTypeInfo() const {
			assert(isCatch());
			return actions_.catchAction.typeInfo;
		}
		
	}
	
}

