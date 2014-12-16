#ifndef LOCIC_SEMANTICANALYSIS_SCOPESTACK_HPP
#define LOCIC_SEMANTICANALYSIS_SCOPESTACK_HPP

#include <string>
#include <vector>

#include <locic/Debug.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/ScopeElement.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		typedef std::vector<ScopeElement> ScopeStack;
		
		class PushScopeElement {
			public:
				inline PushScopeElement(ScopeStack& stack, ScopeElement element)
					: stack_(stack) {
						stack_.push_back(std::move(element));
					}
					
				inline ~PushScopeElement() {
					stack_.pop_back();
				}
				
			private:
				// Non-copyable.
				PushScopeElement(const PushScopeElement&) = delete;
				PushScopeElement& operator=(const PushScopeElement&) = delete;
				
				ScopeStack& stack_;
				
		};
		
		Name getCurrentName(const ScopeStack& scopeStack);
		
		SEM::TypeInstance* lookupParentType(const ScopeStack& scopeStack);
		
		SEM::Function* lookupParentFunction(const ScopeStack& scopeStack);
		
		const SEM::Predicate& lookupRequiresPredicate(const ScopeStack& scopeStack);
		
		const SEM::Type* getParentFunctionReturnType(const ScopeStack& scopeStack);
		
		const SEM::Type* getBuiltInType(const ScopeStack& scopeStack, const std::string& typeName, const std::vector<const SEM::Type*>& templateArgs);
		
	}
	
}

#endif
