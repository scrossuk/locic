#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void AddNamespaceDataAliasValues(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astAliasNode: astNamespaceDataNode->aliases) {
				auto& semAlias = semNamespace.items().at(astAliasNode->name()).alias();
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Alias(semAlias));
				semAlias.setValue(ConvertValue(context, astAliasNode->value()));
			}
			
			for (const auto& astChildNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astChildNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataAliasValues(context, astChildNamespaceNode->data());
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataAliasValues(context, astModuleScopeNode->data);
			}
		}
		
		// Add alias values.
		void AddAliasValuesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataAliasValues(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
