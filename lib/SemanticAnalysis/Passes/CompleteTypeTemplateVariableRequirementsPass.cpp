#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void CompleteTypeAliasTemplateVariableRequirements(Context& context, const AST::Node<AST::TypeAlias>& astTypeAliasNode) {
			const auto typeAlias = context.scopeStack().back().typeAlias();
			
			// Add any requirements in require() specifier.
			auto predicate =
				(!astTypeAliasNode->requireSpecifier.isNull()) ?
					ConvertRequireSpecifier(context, astTypeAliasNode->requireSpecifier) :
					SEM::Predicate::True();
			
			// Add requirements specified inline for template variables.
			for (auto astTemplateVarNode: *(astTypeAliasNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeAlias->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
			 	
			 	const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(semTemplateVar->selfRefType(), semSpecType);
				predicate = SEM::Predicate::And(std::move(predicate), std::move(inlinePredicate));
			}
			
			typeAlias->setRequiresPredicate(std::move(predicate));
		}
		
		void CompleteTypeInstanceTemplateVariableRequirements(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto typeInstance = context.scopeStack().back().typeInstance();
			
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
			for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeInstance->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
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
			for (const auto variantTypeInstance: typeInstance->variants()) {
				if (movePredicate) {
					variantTypeInstance->setMovePredicate(movePredicate->copy());
				}
				variantTypeInstance->setRequiresPredicate(requirePredicate.copy());
			}
			
			if (movePredicate) {
				typeInstance->setMovePredicate(std::move(*movePredicate));
			}
			typeInstance->setRequiresPredicate(std::move(requirePredicate));
		}
		
		void CompleteNamespaceDataTypeTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(&semChildNamespace));
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astNamespaceNode->data);
			}
			
			for (auto astTypeAliasNode: astNamespaceDataNode->typeAliases) {
				auto& semChildTypeAlias = semNamespace->items().at(astTypeAliasNode->name).typeAlias();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeAlias(&semChildTypeAlias));
				CompleteTypeAliasTemplateVariableRequirements(context, astTypeAliasNode);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(&semChildTypeInstance));
				CompleteTypeInstanceTemplateVariableRequirements(context, astTypeInstanceNode);
			}
		}
		
		void CompleteTypeTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astNamespaceNode->data);
			}
		}
		
	}
	
}
