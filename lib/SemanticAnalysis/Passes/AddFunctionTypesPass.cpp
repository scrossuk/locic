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
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astFunctionNode: astNamespaceDataNode->functions) {
				auto& semChildFunction = astFunctionNode->semFunction();
				const auto& name = astFunctionNode->name();
				assert(!name->empty());
				
				if (name->size() == 1) {
					ConvertFunctionDeclType(context, semChildFunction);
				} else {
					const auto searchResult = performSearch(context, name->getPrefix());
					auto& parentTypeInstance = searchResult.typeInstance();
					
					// Push the type instance on the scope stack, since the extension method is
					// effectively within the scope of the type instance.
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(parentTypeInstance));
					ConvertFunctionDeclType(context, semChildFunction);
				}
			}
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataFunctionTypes(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataFunctionTypes(context, astNamespaceNode->data());
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace.items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				for (auto astFunctionNode: *(astTypeInstanceNode->functions)) {
					ConvertFunctionDeclType(context, astFunctionNode->semFunction());
				}
			}
		}
		
		void AddFunctionTypesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataFunctionTypes(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
