#include <locic/AST.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void EvaluateNamespaceStaticAsserts(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astStaticAssertNode: astNamespaceDataNode->staticAsserts) {
				const auto& astPredicateNode = astStaticAssertNode->expression();
				const auto semPredicate = ConvertPredicate(context, astPredicateNode);
				
				const auto evaluateResult = evaluatePredicate(context, semPredicate, SEM::TemplateVarMap());
				assert(evaluateResult);
				if (!*evaluateResult) {
					throw ErrorException(makeString("Static assert predicate '%s' is false, at position %s.",
					                                semPredicate.toString().c_str(),
					                                astPredicateNode.location().toString().c_str()));
				}
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = semNamespace.items().at(astNamespaceNode->name()).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				EvaluateNamespaceStaticAsserts(context, astNamespaceNode->data());
			}
		}
		
		void EvaluateStaticAssertsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				EvaluateNamespaceStaticAsserts(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
