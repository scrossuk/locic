#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Template.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Predicate ConvertPredicateExpr(Context& context, const AST::Node<AST::RequireExpr>& astRequireExprNode) {
			const auto& location = astRequireExprNode.location();
			
			switch (astRequireExprNode->kind()) {
				case AST::RequireExpr::BRACKET: {
					return ConvertPredicateExpr(context, astRequireExprNode->bracketExpr());
				}
				case AST::RequireExpr::TYPESPEC: {
					const auto& typeSpecName = astRequireExprNode->typeSpecName();
					const auto& typeSpecType = astRequireExprNode->typeSpecType();
					
					const auto searchResult = performSearch(context, Name::Relative() + typeSpecName);
					if (!searchResult.isTemplateVar()) {
						throw ErrorException(makeString("Failed to find template var '%s' "
							"in require expression, at position %s.",
							typeSpecName.c_str(),
							location.toString().c_str()));
					}
					
					const auto semTemplateVar = searchResult.templateVar();
					const auto semSpecType = ConvertType(context, typeSpecType);
					
					return SEM::Predicate::Satisfies(semTemplateVar, semSpecType);
				}
				case AST::RequireExpr::AND: {
					auto leftExpr = ConvertPredicateExpr(context, astRequireExprNode->andLeft());
					auto rightExpr = ConvertPredicateExpr(context, astRequireExprNode->andRight());
					return SEM::Predicate::And(std::move(leftExpr), std::move(rightExpr));
				}
			}
			
			throw std::logic_error("Unknown AST RequireExpr kind.");
		}
		
		SEM::Predicate ConvertPredicate(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode) {
			switch (astRequireSpecifierNode->kind()) {
				case AST::RequireSpecifier::NONE:
					// No specifier means it's always true.
					return SEM::Predicate::True();
				case AST::RequireSpecifier::EXPR:
				{
					return ConvertPredicateExpr(context, astRequireSpecifierNode->expr());
				}
			}
			
			throw std::logic_error("Unknown AST RequireSpecifier kind.");
		}
		
		bool evaluateRequiresPredicate(Context& context, const SEM::Predicate& requiresPredicate, const SEM::TemplateVarMap& variableAssignments) {
			switch (requiresPredicate.kind()) {
				case SEM::Predicate::TRUE:
				{
					return true;
				}
				case SEM::Predicate::FALSE:
				{
					return false;
				}
				case SEM::Predicate::AND:
				{
					const auto leftIsTrue = evaluateRequiresPredicate(context, requiresPredicate.andLeft(), variableAssignments);
					const auto rightIsTrue = evaluateRequiresPredicate(context, requiresPredicate.andRight(), variableAssignments);
					return leftIsTrue && rightIsTrue;
				}
				case SEM::Predicate::SATISFIES:
				{
					const auto templateVar = requiresPredicate.satisfiesTemplateVar();
					const auto requireType = requiresPredicate.satisfiesRequirement();
					
					const auto templateValue = variableAssignments.at(templateVar);
					// Some of the requirements can depend on the template values provided.
					const auto substitutedRequireType = requireType->substitute(variableAssignments);
					
					const auto sourceMethodSet = getTypeMethodSet(context, templateValue);
					const auto requireMethodSet = getTypeMethodSet(context, substitutedRequireType);
					
					return methodSetSatisfiesRequirement(sourceMethodSet, requireMethodSet);
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		
		SEM::Predicate simplifyPredicate(const SEM::Predicate& predicate) {
			switch (predicate.kind()) {
				case SEM::Predicate::TRUE:
				case SEM::Predicate::FALSE:
				case SEM::Predicate::SATISFIES:
				{
					return predicate.copy();
				}
				case SEM::Predicate::AND:
				{
					auto left = simplifyPredicate(predicate.andLeft());
					auto right = simplifyPredicate(predicate.andRight());
					if (left.isTrue()) {
						return right;
					} else if (right.isTrue()) {
						return left;
					} else if (left.isFalse() || right.isFalse()) {
						return SEM::Predicate::False();
					} else {
						return SEM::Predicate::And(std::move(left), std::move(right));
					}
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
	}
	
}


