#include <locic/AST.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		class StaticAssertPredicateIsFalseDiag: public Error {
		public:
			StaticAssertPredicateIsFalseDiag() { }
			
			std::string toString() const {
				return "static assert predicate evaluates to false";
			}
			
		};
		
		void EvaluateNamespaceStaticAsserts(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			for (const auto& astStaticAssertNode: astNamespaceDataNode->staticAsserts) {
				const auto& astPredicateNode = astStaticAssertNode->expression();
				const auto semPredicate = ConvertPredicate(context, astPredicateNode);
				
				const auto evaluateResult = evaluatePredicate(context, semPredicate, SEM::TemplateVarMap());
				if (!evaluateResult) {
					context.issueDiag(StaticAssertPredicateIsFalseDiag(),
					                  astPredicateNode.location());
				}
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				EvaluateNamespaceStaticAsserts(context, astNamespaceNode->data());
			}
		}
		
		void EvaluateStaticAssertsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				EvaluateNamespaceStaticAsserts(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
