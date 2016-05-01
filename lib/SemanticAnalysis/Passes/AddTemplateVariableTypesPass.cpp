#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		class TemplateVarHasNonPrimitiveTypeDiag: public Error {
		public:
			TemplateVarHasNonPrimitiveTypeDiag(const String& name,
			                                   const SEM::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("template variable '%s' has non-primitive type '%s'",
				                  name_.c_str(), typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		void AddAliasTemplateVariableTypes(Context& context, const AST::Node<AST::AliasDecl>& astAliasNode) {
			auto& alias = context.scopeStack().back().alias();
			
			// Add types of template variables.
			for (const auto& astTemplateVarNode: *(astAliasNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name();
				const auto semTemplateVar = alias.namedTemplateVariables().at(templateVarName);
				
				const auto& astVarType = astTemplateVarNode->type();
				const auto semVarType = ConvertTemplateVarType(context, astVarType);
				
				if (!semVarType->isPrimitive()) {
					context.issueDiag(TemplateVarHasNonPrimitiveTypeDiag(templateVarName, semVarType),
					                  astTemplateVarNode.location());
				}
				
				semTemplateVar->setType(semVarType);
			}
		}
		
		void AddTypeInstanceTemplateVariableTypes(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			auto& typeInstance = context.scopeStack().back().typeInstance();
			
			// Add types of template variables.
			for (const auto& astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name();
				const auto semTemplateVar = typeInstance.namedTemplateVariables().at(templateVarName);
				
				const auto& astVarType = astTemplateVarNode->type();
				const auto semVarType = ConvertTemplateVarType(context, astVarType);
				
				if (!semVarType->isPrimitive()) {
					context.issueDiag(TemplateVarHasNonPrimitiveTypeDiag(templateVarName, semVarType),
					                  astTemplateVarNode.location());
				}
				
				semTemplateVar->setType(semVarType);
			}
		}
		
		void AddNamespaceDataTypeTemplateVariableTypes(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astModuleScopeNode->data);
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataTypeTemplateVariableTypes(context, astNamespaceNode->data());
			}
			
			for (const auto& astAliasNode: astNamespaceDataNode->aliases) {
				auto& semChildAlias = astAliasNode->alias();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Alias(semChildAlias));
				AddAliasTemplateVariableTypes(context, astAliasNode);
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace.items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				AddTypeInstanceTemplateVariableTypes(context, astTypeInstanceNode);
			}
		}
		
		void AddTemplateVariableTypesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
