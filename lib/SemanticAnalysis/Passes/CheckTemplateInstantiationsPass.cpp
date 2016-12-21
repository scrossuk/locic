#include <locic/AST.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
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
			
			// std::tuple<ScopeStack, AST::TemplateVarMap, const AST::HasRequiresPredicate*, Name, Debug::SourceLocation>
			for (auto& inst: templateInsts) {
				auto& savedScopeStack = inst.scopeStack();
				const auto& variableAssignments = inst.templateVarMap();
				const auto& templatedObject = inst.templatedObject();
				const auto& location = inst.location();
				
				// Swap the current scope stack with the saved stack so we
				// can reproduce the environment of the template instantiation
				// (and then swap them back afterwards).
				SwapScopeStack swapScopeStack(context.scopeStack(), savedScopeStack);
				
				CheckTemplateInstantiation(context,
				                           templatedObject,
				                           variableAssignments,
				                           location);
			}
			
			templateInsts.clear();
			context.setTemplateRequirementsComplete();
		}
		
	}
	
}
