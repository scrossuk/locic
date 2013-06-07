#ifndef LOCIC_CODEGEN_DESTRUCTOR_HPP
#define LOCIC_CODEGEN_DESTRUCTOR_HPP

#include <llvm/Value.h>
#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Function.hpp>

namespace Locic {

	namespace CodeGen {
	
		void genDestructorCall(Function& function, SEM::Type* type, llvm::Value* value);
		
		llvm::Function* genDestructorFunction(Module& module, SEM::Type* parent);
		
		inline void genScopeDestructorCalls(Function& function, const DestructorScope& destructorScope) {
			for (size_t i = 0; i < destructorScope.size(); i++) {
				// Destroy in reverse order.
				const std::pair<SEM::Type*, llvm::Value*> object =
					destructorScope.at((destructorScope.size() - 1) - i);
				genDestructorCall(function, object.first, object.second);
			}
		}
		
		inline void genAllScopeDestructorCalls(Function& function) {
			const std::vector<DestructorScope>& scopeStack =
				function.destructorScopeStack();
			for (size_t i = 0; i < scopeStack.size(); i++) {
				// Destroy scopes in reverse order.
				const DestructorScope& destructorScope =
					scopeStack.at((scopeStack.size() - 1) - i);
				genScopeDestructorCalls(function, destructorScope);
			}
		}
		
		class LifetimeScope {
			public:
				inline LifetimeScope(Function& function)
					: function_(function) {
					function_.destructorScopeStack().push_back(DestructorScope());
				}
				
				inline ~LifetimeScope() {
					genScopeDestructorCalls(function_, function_.destructorScopeStack().back());
					function_.destructorScopeStack().pop_back();
				}
				
			private:
				Function& function_;
			
		};
		
	}
	
}

#endif
