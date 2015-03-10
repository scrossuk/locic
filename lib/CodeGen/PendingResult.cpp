#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	namespace CodeGen {
		
		PendingAction PendingAction::Bind(const SEM::Type* const type) {
			PendingAction action(BIND);
			action.union_.type = type;
			return action;
		}
		
		PendingAction::PendingAction(const Kind argKind)
		: kind_(argKind) { }
		
		PendingAction::Kind PendingAction::kind() const {
			return kind_;
		}
		
		bool PendingAction::isBind() const {
			return kind() == BIND;
		}
		
		const SEM::Type* PendingAction::bindType() const {
			assert(isBind());
			return union_.type;
		}
		
		llvm::Value* PendingAction::resolve(Function& function, llvm::Value* const input) const {
			switch (kind()) {
				case BIND: {
					return genValuePtr(function, input, bindType());
				}
			}
			
			llvm_unreachable("Unknown pending action kind.");
		}
		
		PendingResult::PendingResult(llvm::Value* const value)
		: value_(value) { }
		
		void PendingResult::add(const PendingAction action) {
			actions_.push_back(action);
		}
		
		static llvm::Value* resolveActions(Function& function, llvm::Value* const input, const Array<PendingAction, 10>& actions, const size_t upTo) {
			llvm::Value* result = input;
			for (size_t i = 0; i < upTo; i++) {
				result = actions[i].resolve(function, result);
			}
			return result;
		}
		
		llvm::Value* PendingResult::resolve(Function& function) {
			value_ = resolveActions(function, value_, actions_, actions_.size());
			return value_;
		}
		
		llvm::Value* PendingResult::resolveWithoutBind(Function& function, const SEM::Type* const type) {
			if (!actions_.empty() && actions_.back().isBind()) {
				// The last action is a bind, so we can just do everything
				// else up to it and then return the result from that.
				value_ = resolveActions(function, value_, actions_, actions_.size() - 1);
				Array<PendingAction, 10> newActions;
				newActions.push_back(actions_.back());
				actions_ = std::move(newActions);
				return value_;
			} else {
				// The last action (if any) is not a bind, so presumably a load will be required here.
				value_ = resolveActions(function, value_, actions_, actions_.size());
				auto& module = function.module();
				auto& builder = function.getBuilder();
				return genMoveLoad(function, builder.CreatePointerCast(value_, genPointerType(module, type)), type);
			}
		}
		
		llvm::Value* PendingResult::resolveWithoutBindRaw(Function& function, llvm::Type* const type) {
			if (!actions_.empty() && actions_.back().isBind()) {
				// The last action is a bind, so we can just do everything
				// else up to it and then return the result from that.
				value_ = resolveActions(function, value_, actions_, actions_.size() - 1);
				Array<PendingAction, 10> newActions;
				newActions.push_back(actions_.back());
				actions_ = std::move(newActions);
				return value_;
			} else {
				// The last action (if any) is not a bind, so presumably a load will be required here.
				value_ = resolveActions(function, value_, actions_, actions_.size());
				auto& builder = function.getBuilder();
				return builder.CreateLoad(builder.CreatePointerCast(value_, type->getPointerTo()));
			}
		}
		
	}
	
}
