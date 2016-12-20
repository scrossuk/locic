#include <locic/AST.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void CompleteAliasTemplateVariableRequirements(Context& context, const AST::Node<AST::Alias>& aliasNode) {
			auto& alias = context.scopeStack().back().alias();
			
			// Add any requirements in require() specifier.
			auto predicate =
				(!aliasNode->requireSpecifier().isNull()) ?
					ConvertRequireSpecifier(context, aliasNode->requireSpecifier()) :
					SEM::Predicate::True();
			
			// Add requirements specified inline for template variables.
			for (const auto& templateVarNode: *(aliasNode->templateVariableDecls())) {
				TypeResolver typeResolver(context);
				auto templateVarTypePredicate =
					typeResolver.getTemplateVarTypePredicate(templateVarNode->typeDecl(),
					                                         *templateVarNode);
				predicate = SEM::Predicate::And(std::move(predicate),
				                                std::move(templateVarTypePredicate));
				
				auto& astSpecType = templateVarNode->specType();
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
				
				const auto semSpecType = typeResolver.resolveType(astSpecType);
				
				// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(templateVarNode->selfRefType(), semSpecType);
				predicate = SEM::Predicate::And(std::move(predicate), std::move(inlinePredicate));
			}
			
			alias.setRequiresPredicate(std::move(predicate));
		}
		
		void CompleteTypeInstanceTemplateVariableRequirements(Context& context, const AST::Node<AST::TypeInstance>& typeInstanceNode) {
			// Add any requirements in move() specifier, if any is provided.
			auto movePredicate =
				(!typeInstanceNode->moveSpecifier.isNull() && typeInstanceNode->moveSpecifier->isExpr()) ?
					make_optional(ConvertRequireSpecifier(context, typeInstanceNode->moveSpecifier)) :
					None;
			
			// Add any requirements in require() specifier.
			auto requirePredicate =
				(!typeInstanceNode->requireSpecifier.isNull()) ?
					ConvertRequireSpecifier(context, typeInstanceNode->requireSpecifier) :
					SEM::Predicate::True();
			
			// Add requirements specified inline for template variables.
			for (const auto& templateVarNode: *(typeInstanceNode->templateVariableDecls)) {
				TypeResolver typeResolver(context);
				auto templateVarTypePredicate =
					typeResolver.getTemplateVarTypePredicate(templateVarNode->typeDecl(),
					                                         *templateVarNode);
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate),
				                                       std::move(templateVarTypePredicate));
				
				auto& astSpecType = templateVarNode->specType();
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
				
				const auto semSpecType = typeResolver.resolveType(astSpecType);
			 	
			 	// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(templateVarNode->selfRefType(), semSpecType);
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), std::move(inlinePredicate));
			}
			
			// Copy requires predicate to all variant types.
			for (const auto variantTypeInstance: typeInstanceNode->variants()) {
				if (movePredicate) {
					variantTypeInstance->setMovePredicate(movePredicate->copy());
				}
				variantTypeInstance->setRequiresPredicate(requirePredicate.copy());
			}
			
			if (movePredicate) {
				typeInstanceNode->setMovePredicate(std::move(*movePredicate));
			}
			typeInstanceNode->setRequiresPredicate(std::move(requirePredicate));
		}
		
		void CompleteNamespaceDataTypeTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataTypeTemplateVariableRequirements(context,
				                                                      astModuleScopeNode->data());
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astNamespaceNode->data());
			}
			
			for (const auto& aliasNode: astNamespaceDataNode->aliases) {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Alias(*aliasNode));
				CompleteAliasTemplateVariableRequirements(context, aliasNode);
			}
			
			for (const auto& typeInstanceNode: astNamespaceDataNode->typeInstances) {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(*typeInstanceNode));
				CompleteTypeInstanceTemplateVariableRequirements(context,typeInstanceNode);
			}
		}
		
		void CompleteTypeTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
