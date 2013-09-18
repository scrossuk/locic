#ifndef LOCIC_CODEGEN_DESTRUCTOR_HPP
#define LOCIC_CODEGEN_DESTRUCTOR_HPP

#include <llvm/Value.h>
#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Function.hpp>

namespace Locic {

	namespace CodeGen {
	
		void genDestructorCall(Function& function, SEM::Type* type, llvm::Value* value);
		
		llvm::Function* genDestructorFunction(Module& module, SEM::Type* parent);
		
		void genScopeDestructorCalls(Function& function, const DestructorScope& destructorScope, size_t scopeId);
		
		void genAllScopeDestructorCalls(Function& function);
		
		class LifetimeScope {
			public:
				LifetimeScope(Function& function);
				~LifetimeScope();
				
			private:
				Function& function_;
			
		};
		
	}
	
}

#endif
