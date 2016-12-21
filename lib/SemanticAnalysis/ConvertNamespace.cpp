#include <cstddef>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>

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
		
		void ConvertNamespaceFunctionDef(Context& context, const AST::Node<AST::Function>& function) {
			const auto& name = function->nameDecl();
			
			if (name->size() == 1) {
				// Normal namespace function.
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(*function));
				ConvertFunctionDef(context, function);
			} else {
				// Extension method.
				const auto searchResult = performSearch(context, name->getPrefix());
				if (!searchResult.isTypeInstance()) {
					return;
				}
				
				auto& typeInstance = searchResult.typeInstance();
				
				PushScopeElement pushTypeInstance(context.scopeStack(), ScopeElement::TypeInstance(typeInstance));
				PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(*function));
				ConvertFunctionDef(context, function);
			}
		}
		
		void ConvertNamespaceData(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			for (const auto& function: astNamespaceDataNode->functions) {
				ConvertNamespaceFunctionDef(context, function);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				ConvertNamespaceData(context, astModuleScopeNode->data());
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				ConvertNamespaceData(context, astNamespaceNode->data());
			}
			
			for (auto& typeInstanceNode: astNamespaceDataNode->typeInstances) {
				{
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(*typeInstanceNode));
					ConvertTypeInstance(context, typeInstanceNode);
				}
				
				for (auto& variantNode: *(typeInstanceNode->variantDecls)) {
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(*variantNode));
					ConvertTypeInstance(context, variantNode);
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

