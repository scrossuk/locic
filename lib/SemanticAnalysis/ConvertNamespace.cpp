#include <cstddef>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/ConvertTypeInstance.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void ConvertNamespaceData(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astFunctionNode: astNamespaceDataNode->functions) {
				const auto semChildFunction = semNamespace->items().at(astFunctionNode->name()).function();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(semChildFunction));
				ConvertFunctionDef(context, astFunctionNode);
			}
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				ConvertNamespaceData(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				ConvertNamespaceData(context, astNamespaceNode->data);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				{
					const auto semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
					
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
					ConvertTypeInstance(context, astTypeInstanceNode);
				}
				
				for (const auto& astVariantNode: *(astTypeInstanceNode->variants)) {
					const auto semVariantTypeInstance = semNamespace->items().at(astVariantNode->name).typeInstance();
					
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semVariantTypeInstance));
					ConvertTypeInstance(context, astTypeInstanceNode);
				}
			}
		}
		
		void ConvertNamespace(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				ConvertNamespaceData(context, astNamespaceNode->data);
			}
		}
		
	}
	
}

