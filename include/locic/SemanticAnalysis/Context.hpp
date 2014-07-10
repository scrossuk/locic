#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <locic/Debug.hpp>

#include <locic/SEM/Context.hpp>

#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class Context {
			public:
				Context(Debug::Module& pDebugModule, SEM::Context& pSemContext);
				
				Debug::Module& debugModule();
				
				ScopeStack& scopeStack();
				const ScopeStack& scopeStack() const;
				
				SEM::Context& semContext();
			
			private:
				// Non-copyable.
				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;
				
				Debug::Module& debugModule_;
				ScopeStack scopeStack_;
				SEM::Context& semContext_;
				
		};
		
		SEM::Value* getSelfValue(Context& context, const Debug::SourceLocation& location);
		
	}
	
}

#endif
