#include <cstddef>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/ConvertTypeInstance.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void ConvertNamespaceFunctionDef(Context& context, const AST::Node<AST::Function>& astFunctionNode) {
			const auto& name = astFunctionNode->name();
			
			if (name->size() == 1) {
				// Normal namespace function.
				auto& semChildFunction = astFunctionNode->semFunction();
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(semChildFunction));
				ConvertFunctionDef(context, astFunctionNode);
			} else {
				// Extension method.
				const auto searchResult = performSearch(context, name->getPrefix());
				if (!searchResult.isTypeInstance()) {
					return;
				}
				
				auto& semTypeInstance = searchResult.typeInstance();
				auto& semChildFunction = astFunctionNode->semFunction();
				
				PushScopeElement pushTypeInstance(context.scopeStack(), ScopeElement::TypeInstance(semTypeInstance));
				PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(semChildFunction));
				ConvertFunctionDef(context, astFunctionNode);
			}
		}
		
		void ConvertNamespaceData(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astFunctionNode: astNamespaceDataNode->functions) {
				ConvertNamespaceFunctionDef(context, astFunctionNode);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				ConvertNamespaceData(context, astModuleScopeNode->data());
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				ConvertNamespaceData(context, astNamespaceNode->data());
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				{
					auto& semChildTypeInstance = astTypeInstanceNode->semTypeInstance();
					
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
					ConvertTypeInstance(context, astTypeInstanceNode);
				}
				
				for (const auto& astVariantNode: *(astTypeInstanceNode->variants)) {
					auto& semVariantTypeInstance = semNamespace.items().at(astVariantNode->name).typeInstance();
					
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semVariantTypeInstance));
					ConvertTypeInstance(context, astTypeInstanceNode);
				}
			}
		}
		
		void ConvertNamespace(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				ConvertNamespaceData(context, astNamespaceNode->data());
			}
		}
		
	}
	
}

