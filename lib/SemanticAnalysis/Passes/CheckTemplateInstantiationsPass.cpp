#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TemplateInst.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		class SwapScopeStack {
			public:
				SwapScopeStack(ScopeStack& first,
					ScopeStack& second)
					: first_(first), second_(second) {
						std::swap(first_, second_);
					}
					
				~SwapScopeStack() {
					std::swap(first_, second_);
				}
				
			private:
				ScopeStack& first_;
				ScopeStack& second_;
				
		};
		
		void CheckTemplateInstantiationsPass(Context& context) {
			auto& templateInsts = context.templateInstantiations();
			
			// std::tuple<ScopeStack, SEM::TemplateVarMap, const SEM::HasRequiresPredicate*, Name, Debug::SourceLocation>
			for (auto& inst: templateInsts) {
				auto& savedScopeStack = inst.scopeStack();
				const auto& variableAssignments = inst.templateVarMap();
				const auto hasRequiresPredicate = inst.templatedObject();
				const auto& parentName = inst.name();
				const auto& location = inst.location();
				
				const auto& requiresPredicate = hasRequiresPredicate->requiresPredicate();
				
				// Swap the current scope stack with the saved stack so we
				// can reproduce the environment of the template instantiation
				// (and then swap them back afterwards).
				SwapScopeStack swapScopeStack(context.scopeStack(), savedScopeStack);
				
				// Conservatively assume require predicate is not satisified if result is undetermined.
				const bool satisfiesRequiresDefault = false;
				
				if (!evaluatePredicateWithDefault(context, requiresPredicate, variableAssignments, satisfiesRequiresDefault)) {
					throw ErrorException(makeString("Template arguments do not satisfy "
						"requires predicate '%s' of function or type '%s' at position %s.",
						requiresPredicate.substitute(variableAssignments).toString().c_str(),
						parentName.toString().c_str(),
						location.toString().c_str()));
				}
			}
			
			templateInsts.clear();
			context.setTemplateRequirementsComplete();
		}
		
	}
	
}
