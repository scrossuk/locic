#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void AddNamespaceDataFunctionTypes(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			for (auto& function: astNamespaceDataNode->functions) {
				const auto& name = function->nameDecl();
				assert(!name->empty());
				
				if (name->size() == 1) {
					ConvertFunctionDeclType(context, function);
				} else {
					const auto searchResult = performSearch(context, name->getPrefix());
					if (!searchResult.isTypeInstance()) {
						continue;
					}
					
					auto& parentTypeInstance = searchResult.typeInstance();
					
					// Push the type instance on the scope stack, since the extension method is
					// effectively within the scope of the type instance.
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(parentTypeInstance));
					ConvertFunctionDeclType(context, function);
				}
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataFunctionTypes(context, astModuleScopeNode->data());
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataFunctionTypes(context, astNamespaceNode->data());
			}
			
			for (const auto& typeInstanceNode: astNamespaceDataNode->typeInstances) {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(*typeInstanceNode));
				for (auto& function: *(typeInstanceNode->functionDecls)) {
					ConvertFunctionDeclType(context, function);
				}
			}
		}
		
		void AddFunctionTypesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataFunctionTypes(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
