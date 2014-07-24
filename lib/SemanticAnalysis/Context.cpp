#include <locic/Debug.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Context::Context(Debug::Module& pDebugModule, SEM::Context& pSemContext)
			: debugModule_(pDebugModule), semContext_(pSemContext) { }
		
		Debug::Module& Context::debugModule() {
			return debugModule_;
		}
		
		ScopeStack& Context::scopeStack() {
			return scopeStack_;
		}
		
		const ScopeStack& Context::scopeStack() const {
			return scopeStack_;
		}
		
		SEM::Context& Context::semContext() {
			return semContext_;
		}
		
		std::vector<TemplateInstTuple>& Context::templateInstantiations() {
			return templateInstantiations_;
		}
		
		SEM::Value* getSelfValue(Context& context, const Debug::SourceLocation& location) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			const auto thisFunction = lookupParentFunction(context.scopeStack());
			
			if (thisTypeInstance == nullptr) {
				throw ErrorException(makeString("Cannot access 'self' in non-method at %s.", location.toString().c_str()));
			}
			
			if (thisFunction->isStaticMethod()) {
				throw ErrorException(makeString("Cannot access 'self' in static method at %s.",
					location.toString().c_str()));
			}
			
			const auto selfType = thisTypeInstance->selfType();
			const auto selfConstType = thisFunction->isConstMethod() ? selfType->createConstType() : selfType;
			return createSelfRef(context, selfConstType);
		}
		
	}
	
}

