#include <locic/AST.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void CompleteAliasTemplateVariableRequirements(Context& context, const AST::Node<AST::AliasDecl>& astAliasNode) {
			auto& alias = context.scopeStack().back().alias();
			
			// Add any requirements in require() specifier.
			auto predicate =
				(!astAliasNode->requireSpecifier().isNull()) ?
					ConvertRequireSpecifier(context, astAliasNode->requireSpecifier()) :
					SEM::Predicate::True();
			
			// Add requirements specified inline for template variables.
			for (const auto& astTemplateVarNode: *(astAliasNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name();
				const auto semTemplateVar = alias.namedTemplateVariables().at(templateVarName);
				
				auto templateVarTypePredicate =
					getTemplateVarTypePredicate(context, astTemplateVarNode->type(),
					                            *semTemplateVar);
				predicate = SEM::Predicate::And(std::move(predicate),
				                                std::move(templateVarTypePredicate));
				
				const auto& astSpecType = astTemplateVarNode->specType();
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
			 	
			 	const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(semTemplateVar->selfRefType(), semSpecType);
				predicate = SEM::Predicate::And(std::move(predicate), std::move(inlinePredicate));
			}
			
			alias.setRequiresPredicate(std::move(predicate));
		}
		
		void CompleteTypeInstanceTemplateVariableRequirements(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			auto& typeInstance = context.scopeStack().back().typeInstance();
			
			// Add any requirements in move() specifier, if any is provided.
			auto movePredicate =
				(!astTypeInstanceNode->moveSpecifier.isNull() && astTypeInstanceNode->moveSpecifier->isExpr()) ?
					make_optional(ConvertRequireSpecifier(context, astTypeInstanceNode->moveSpecifier)) :
					None;
			
			// Add any requirements in require() specifier.
			auto requirePredicate =
				(!astTypeInstanceNode->requireSpecifier.isNull()) ?
					ConvertRequireSpecifier(context, astTypeInstanceNode->requireSpecifier) :
					SEM::Predicate::True();
			
			// Add requirements specified inline for template variables.
			for (const auto& astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name();
				const auto semTemplateVar = typeInstance.namedTemplateVariables().at(templateVarName);
				
				auto templateVarTypePredicate =
					getTemplateVarTypePredicate(context, astTemplateVarNode->type(),
					                            *semTemplateVar);
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate),
				                                       std::move(templateVarTypePredicate));
				
				const auto& astSpecType = astTemplateVarNode->specType();
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
				
				const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(semTemplateVar->selfRefType(), semSpecType);
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), std::move(inlinePredicate));
			}
			
			// Copy requires predicate to all variant types.
			for (const auto variantTypeInstance: typeInstance.variants()) {
				if (movePredicate) {
					variantTypeInstance->setMovePredicate(movePredicate->copy());
				}
				variantTypeInstance->setRequiresPredicate(requirePredicate.copy());
			}
			
			if (movePredicate) {
				typeInstance.setMovePredicate(std::move(*movePredicate));
			}
			typeInstance.setRequiresPredicate(std::move(requirePredicate));
		}
		
		void CompleteNamespaceDataTypeTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astModuleScopeNode->data);
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astNamespaceNode->data());
			}
			
			for (const auto& astAliasNode: astNamespaceDataNode->aliases) {
				auto& semChildAlias = astAliasNode->alias();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Alias(semChildAlias));
				CompleteAliasTemplateVariableRequirements(context, astAliasNode);
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace.items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				CompleteTypeInstanceTemplateVariableRequirements(context, astTypeInstanceNode);
			}
		}
		
		void CompleteTypeTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
