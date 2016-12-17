#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>

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
			// Add types of template variables.
			for (const auto& templateVarNode: *(astAliasNode->templateVariables())) {
				auto& astVarType = templateVarNode->typeDecl();
				const auto semVarType = TypeResolver(context).resolveTemplateVarType(astVarType);
				
				if (!semVarType->isPrimitive()) {
					const auto& templateVarName = templateVarNode->name();
					context.issueDiag(TemplateVarHasNonPrimitiveTypeDiag(templateVarName, semVarType),
					                  templateVarNode.location());
				}
				
				templateVarNode->setType(semVarType);
			}
		}
		
		void AddTypeInstanceTemplateVariableTypes(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			// Add types of template variables.
			for (const auto& templateVarNode: *(astTypeInstanceNode->templateVariables)) {
				auto& astVarType = templateVarNode->typeDecl();
				const auto semVarType = TypeResolver(context).resolveTemplateVarType(astVarType);
				
				if (!semVarType->isPrimitive()) {
					const auto& templateVarName = templateVarNode->name();
					context.issueDiag(TemplateVarHasNonPrimitiveTypeDiag(templateVarName, semVarType),
					                  templateVarNode.location());
				}
				
				templateVarNode->setType(semVarType);
			}
		}
		
		void AddNamespaceDataTypeTemplateVariableTypes(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astModuleScopeNode->data());
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
				auto& semChildTypeInstance = astTypeInstanceNode->semTypeInstance();
				
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
