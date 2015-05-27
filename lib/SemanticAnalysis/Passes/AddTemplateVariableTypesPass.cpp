#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void AddTypeAliasTemplateVariableTypes(Context& context, const AST::Node<AST::TypeAlias>& astTypeAliasNode) {
			const auto typeAlias = context.scopeStack().back().typeAlias();
			
			// Add types of template variables.
			for (auto astTemplateVarNode: *(astTypeAliasNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeAlias->namedTemplateVariables().at(templateVarName);
				
				const auto& astVarType = astTemplateVarNode->varType;
				const auto semVarType = ConvertType(context, astVarType);
				
				if (!semVarType->isBuiltInBool() && !semVarType->isBuiltInTypename()) {
					throw ErrorException(makeString("Template variable '%s' in type alias '%s' has invalid type '%s', at position %s.",
						templateVarName.c_str(), typeAlias->name().toString().c_str(),
						semVarType->toString().c_str(),
						astTemplateVarNode.location().toString().c_str()));
				}
				
				semTemplateVar->setType(semVarType);
			}
		}
		
		void AddTypeInstanceTemplateVariableTypes(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto typeInstance = context.scopeStack().back().typeInstance();
			
			// Add types of template variables.
			for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeInstance->namedTemplateVariables().at(templateVarName);
				
				const auto& astVarType = astTemplateVarNode->varType;
				const auto semVarType = ConvertType(context, astVarType);
				
				if (!semVarType->isBuiltInBool() && !semVarType->isBuiltInTypename()) {
					throw ErrorException(makeString("Template variable '%s' in type '%s' has invalid type '%s', at position %s.",
						templateVarName.c_str(), typeInstance->name().toString().c_str(),
						semVarType->toString().c_str(),
						astTemplateVarNode.location().toString().c_str()));
				}
				
				semTemplateVar->setType(semVarType);
			}
		}
		
		void AddNamespaceDataTypeTemplateVariableTypes(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(&semChildNamespace));
				AddNamespaceDataTypeTemplateVariableTypes(context, astNamespaceNode->data);
			}
			
			for (auto astTypeAliasNode: astNamespaceDataNode->typeAliases) {
				auto& semChildTypeAlias = semNamespace->items().at(astTypeAliasNode->name).typeAlias();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeAlias(&semChildTypeAlias));
				AddTypeAliasTemplateVariableTypes(context, astTypeAliasNode);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(&semChildTypeInstance));
				AddTypeInstanceTemplateVariableTypes(context, astTypeInstanceNode);
			}
		}
		
		void AddTemplateVariableTypesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astNamespaceNode->data);
			}
		}
		
	}
	
}
