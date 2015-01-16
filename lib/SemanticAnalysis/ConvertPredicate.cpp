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
		
		SEM::Predicate ConvertPredicate(Context& context, const AST::Node<AST::Predicate>& astPredicateNode) {
			const auto& location = astPredicateNode.location();
			
			switch (astPredicateNode->kind()) {
				case AST::Predicate::BRACKET: {
					return ConvertPredicate(context, astPredicateNode->bracketExpr());
				}
				case AST::Predicate::TYPESPEC: {
					const auto& typeSpecName = astPredicateNode->typeSpecName();
					const auto& typeSpecType = astPredicateNode->typeSpecType();
					
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
				case AST::Predicate::VARIABLE: {
					const auto& variableName = astPredicateNode->variableName();
					
					const auto searchResult = performSearch(context, Name::Relative() + variableName);
					if (!searchResult.isTemplateVar()) {
						throw ErrorException(makeString("Failed to find template var '%s' "
							"in require expression, at position %s.",
							variableName.c_str(),
							location.toString().c_str()));
					}
					
					return SEM::Predicate::Variable(searchResult.templateVar());
				}
				case AST::Predicate::AND: {
					auto leftExpr = ConvertPredicate(context, astPredicateNode->andLeft());
					auto rightExpr = ConvertPredicate(context, astPredicateNode->andRight());
					return SEM::Predicate::And(std::move(leftExpr), std::move(rightExpr));
				}
			}
			
			throw std::logic_error("Unknown AST Predicate kind.");
		}
		
		SEM::Predicate ConvertConstSpecifier(Context& context, const AST::Node<AST::ConstSpecifier>& astConstSpecifierNode) {
			switch (astConstSpecifierNode->kind()) {
				case AST::ConstSpecifier::NONE:
					// No specifier means it's false (i.e. always not const).
					return SEM::Predicate::False();
				case AST::ConstSpecifier::CONST:
					// 'const' means it's true (i.e. always const).
					return SEM::Predicate::True();
				case AST::ConstSpecifier::MUTABLE:
					// 'mutable' means it's false (i.e. always not const).
					return SEM::Predicate::False();
				case AST::ConstSpecifier::EXPR:
				{
					return ConvertPredicate(context, astConstSpecifierNode->predicate());
				}
			}
			
			throw std::logic_error("Unknown AST ConstSpecifier kind.");
		}
		
		SEM::Predicate ConvertRequireSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode) {
			switch (astRequireSpecifierNode->kind()) {
				case AST::RequireSpecifier::NONE:
					// No specifier means it's always true.
					return SEM::Predicate::True();
				case AST::RequireSpecifier::EXPR:
				{
					return ConvertPredicate(context, astRequireSpecifierNode->expr());
				}
			}
			
			throw std::logic_error("Unknown AST RequireSpecifier kind.");
		}
		
		bool evaluatePredicate(Context& context, const SEM::Predicate& predicate, const SEM::TemplateVarMap& variableAssignments) {
			switch (predicate.kind()) {
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
					const auto leftIsTrue = evaluatePredicate(context, predicate.andLeft(), variableAssignments);
					const auto rightIsTrue = evaluatePredicate(context, predicate.andRight(), variableAssignments);
					return leftIsTrue && rightIsTrue;
				}
				case SEM::Predicate::SATISFIES:
				{
					const auto templateVar = predicate.satisfiesTemplateVar();
					const auto requireType = predicate.satisfiesRequirement();
					
					const auto templateValue = variableAssignments.at(templateVar)->resolveAliases();
					if (templateValue->isAuto()) {
						// Presumably this will work.
						return true;
					}
					
					// Some of the requirements can depend on the template values provided.
					const auto substitutedRequireType = requireType->substitute(variableAssignments);
					
					const auto sourceMethodSet = getTypeMethodSet(context, templateValue);
					const auto requireMethodSet = getTypeMethodSet(context, substitutedRequireType);
					
					return methodSetSatisfiesRequirement(sourceMethodSet, requireMethodSet);
				}
				case SEM::Predicate::VARIABLE:
				{
					const auto templateVar = predicate.variableTemplateVar();
					
					const auto templateValue = variableAssignments.at(templateVar);
					
					// TODO: need to evaluate template value as boolean!
					(void) templateValue;
					
					return true;
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		
		SEM::Predicate simplifyPredicate(const SEM::Predicate& predicate) {
			switch (predicate.kind()) {
				case SEM::Predicate::TRUE:
				case SEM::Predicate::FALSE:
				case SEM::Predicate::SATISFIES:
				case SEM::Predicate::VARIABLE:
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


