#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void CompleteFunctionTemplateVariableRequirements(Context& context, AST::Node<AST::Function>& astFunctionNode,
		                                                  const SEM::Predicate& parentRequiresPredicate) {
			auto& function = context.scopeStack().back().function();
			
			// Add any requirements specified by parent.
			auto predicate = parentRequiresPredicate.copy();
			
			// Add previous requirements added by default methods.
			predicate = SEM::Predicate::And(std::move(predicate),
			                                function.requiresPredicate().copy());
			
			// Add any requirements in require() specifier.
			if (!astFunctionNode->requireSpecifier().isNull()) {
				predicate = SEM::Predicate::And(std::move(predicate), ConvertRequireSpecifier(context, astFunctionNode->requireSpecifier()));
			}
			
			// Add requirements specified inline for template variables.
			for (const auto& templateVarNode: *(astFunctionNode->templateVariableDecls())) {
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
			
			function.setRequiresPredicate(std::move(predicate));
		}
		
		void CompleteNamespaceDataFunctionTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			for (auto& function: astNamespaceDataNode->functions) {
				const auto& name = function->nameDecl();
				assert(!name->empty());
				
				if (name->size() == 1) {
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(*function));
					CompleteFunctionTemplateVariableRequirements(context, function, SEM::Predicate::True());
				} else {
					const auto searchResult = performSearch(context, name->getPrefix());
					if (!searchResult.isTypeInstance()) {
						continue;
					}
					
					auto& parentTypeInstance = searchResult.typeInstance();
					
					// Push the type instance on the scope stack, since the extension method is
					// effectively within the scope of the type instance.
					PushScopeElement pushTypeInstance(context.scopeStack(), ScopeElement::TypeInstance(parentTypeInstance));
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(*function));
					
					CompleteFunctionTemplateVariableRequirements(context, function, parentTypeInstance.requiresPredicate());
				}
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context,
				                                                          astModuleScopeNode->data());
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushNamespace(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context, astNamespaceNode->data());
			}
			
			for (const auto& typeInstanceNode: astNamespaceDataNode->typeInstances) {
				PushScopeElement pushTypeInstance(context.scopeStack(), ScopeElement::TypeInstance(*typeInstanceNode));
				for (auto& function: *(typeInstanceNode->functionDecls)) {
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(*function));
					CompleteFunctionTemplateVariableRequirements(context, function, typeInstanceNode->requiresPredicate());
				}
			}
		}
		
		void CompleteFunctionTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context, astNamespaceNode->data());
			}
		}
		
	}
	
}
