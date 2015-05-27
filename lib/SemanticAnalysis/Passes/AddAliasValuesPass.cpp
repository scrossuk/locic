#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void AddNamespaceDataAliasValues(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astChildNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = semNamespace->items().at(astChildNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(&semChildNamespace));
				AddNamespaceDataAliasValues(context, astChildNamespaceNode->data);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataAliasValues(context, astModuleScopeNode->data);
			}
			
			for (const auto& astTypeAliasNode: astNamespaceDataNode->typeAliases) {
				auto& semTypeAlias = semNamespace->items().at(astTypeAliasNode->name).typeAlias();
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeAlias(&semTypeAlias));
				semTypeAlias.setValue(ConvertType(context, astTypeAliasNode->value));
			}
		}
		
		// Add alias values.
		void AddAliasValuesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataAliasValues(context, astNamespaceNode->data);
			}
		}
		
	}
	
}
