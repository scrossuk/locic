#ifndef LOCIC_CODEGEN_SCOPEEXITACTIONS_HPP
#define LOCIC_CODEGEN_SCOPEEXITACTIONS_HPP

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		void performScopeExitAction(Function& function, size_t position, bool isExceptionState, bool isRethrow);
		
		void genAllScopeExitActions(Function& function, bool isExceptionState = false, bool isRethrow = false);
		
		class ScopeLifetime {
			public:
				ScopeLifetime(Function& function);
				~ScopeLifetime();
				
			private:
				Function& function_;
			
		};
		
		class StatementLifetime {
			public:
				StatementLifetime(Function& function);
				~StatementLifetime();
				
			private:
				Function& function_;
			
		};
		
	}
	
}

#endif
