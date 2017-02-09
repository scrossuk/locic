#include <locic/AST.hpp>
#include <locic/Frontend/OptionalDiag.hpp>
#include <locic/AST/Predicate.hpp>
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
				const auto astPredicate = ConvertPredicate(context, astPredicateNode);
				
				auto evaluateResult = evaluatePredicate(context, astPredicate, AST::TemplateVarMap());
				if (evaluateResult.failed()) {
					context.issueDiag(StaticAssertPredicateIsFalseDiag(),
					                  astPredicateNode.location(),
					                  std::move(evaluateResult));
				}
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& astChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(astChildNamespace));
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
