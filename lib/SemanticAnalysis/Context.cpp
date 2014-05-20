#include <locic/Debug.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Context::Context(Debug::Module& pDebugModule)
			: debugModule_(pDebugModule) { }
		
		Debug::Module& Context::debugModule() {
			return debugModule_;
		}
		
		ScopeStack& Context::scopeStack() {
			return scopeStack_;
		}
		
		const ScopeStack& Context::scopeStack() const {
			return scopeStack_;
		}
		
	}
	
}

