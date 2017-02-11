#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		class TemplateVarHasNonPrimitiveTypeDiag: public ErrorDiag {
		public:
			TemplateVarHasNonPrimitiveTypeDiag(const String& name,
			                                   const AST::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("template variable '%s' has non-primitive type '%s'",
				                  name_.c_str(), typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		void AddAliasTemplateVariableTypes(Context& context, const AST::Node<AST::Alias>& aliasNode) {
			// Add types of template variables.
			for (const auto& templateVarNode: *(aliasNode->templateVariableDecls())) {
				auto& varTypeDecl = templateVarNode->typeDecl();
				const auto varType = TypeResolver(context).resolveTemplateVarType(varTypeDecl);
				
				if (!varType->isPrimitive()) {
					const auto& templateVarName = templateVarNode->name();
					context.issueDiag(TemplateVarHasNonPrimitiveTypeDiag(templateVarName, varType),
					                  templateVarNode.location());
				}
				
				templateVarNode->setType(varType);
			}
		}
		
		void AddTypeInstanceTemplateVariableTypes(Context& context, const AST::Node<AST::TypeInstance>& typeInstanceNode) {
			// Add types of template variables.
			for (const auto& templateVarNode: *(typeInstanceNode->templateVariableDecls)) {
				auto& varTypeDecl = templateVarNode->typeDecl();
				const auto varType = TypeResolver(context).resolveTemplateVarType(varTypeDecl);
				
				if (!varType->isPrimitive()) {
					const auto& templateVarName = templateVarNode->name();
					context.issueDiag(TemplateVarHasNonPrimitiveTypeDiag(templateVarName, varType),
					                  templateVarNode.location());
				}
				
				templateVarNode->setType(varType);
			}
		}
		
		void AddNamespaceDataTypeTemplateVariableTypes(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astModuleScopeNode->data());
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& astChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(astChildNamespace));
				AddNamespaceDataTypeTemplateVariableTypes(context, astNamespaceNode->data());
			}
			
			for (const auto& aliasNode: astNamespaceDataNode->aliases) {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Alias(*aliasNode));
				AddAliasTemplateVariableTypes(context, aliasNode);
			}
			
			for (const auto& typeInstanceNode: astNamespaceDataNode->typeInstances) {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(*typeInstanceNode));
				AddTypeInstanceTemplateVariableTypes(context, typeInstanceNode);
			}
		}
		
		void AddTemplateVariableTypesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
