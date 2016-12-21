#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Debug.hpp>
#include <locic/Frontend/OptionalDiag.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/MethodSetSatisfies.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		class PredicateAliasNotBoolDiag: public Error {
		public:
			PredicateAliasNotBoolDiag(const Name& name, const AST::Type* const type)
			: name_(name.copy()), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("alias '%s' has non-boolean type '%s' and "
				                  "therefore cannot be used in predicate",
				                  name_.toString(/*addPrefix=*/false).c_str(),
				                  typeString_.c_str());
			}
			
		private:
			Name name_;
			std::string typeString_;
			
		};
		
		class PredicateTemplateVarNotBoolDiag: public Error {
		public:
			PredicateTemplateVarNotBoolDiag(const Name& name, const AST::Type* const type)
			: name_(name.copy()), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("template variable '%s' has non-boolean type '%s' "
				                  "and therefore cannot be used in predicate",
				                  name_.toString(/*addPrefix=*/false).c_str(),
				                  typeString_.c_str());
			}
			
		private:
			Name name_;
			std::string typeString_;
			
		};
		
		class InvalidSymbolInPredicateDiag: public Error {
		public:
			InvalidSymbolInPredicateDiag(const Name& name)
			: name_(name.copy()) { }
			
			std::string toString() const {
				return makeString("symbol '%s' cannot be used in predicate",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class UnknownSymbolInPredicateDiag: public Error {
		public:
			UnknownSymbolInPredicateDiag(const Name& name)
			: name_(name.copy()) { }
			
			std::string toString() const {
				return makeString("unknown symbol '%s' cannot be used in predicate",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		SEM::Predicate ConvertPredicate(Context& context, const AST::Node<AST::PredicateDecl>& astPredicateNode) {
			const auto& location = astPredicateNode.location();
			
			switch (astPredicateNode->kind()) {
				case AST::PredicateDecl::TRUE: {
					return SEM::Predicate::True();
				}
				case AST::PredicateDecl::FALSE: {
					return SEM::Predicate::False();
				}
				case AST::PredicateDecl::BRACKET: {
					return ConvertPredicate(context, astPredicateNode->bracketExpr());
				}
				case AST::PredicateDecl::TYPESPEC: {
					auto& typeSpecType = astPredicateNode->typeSpecType();
					auto& typeSpecRequireType = astPredicateNode->typeSpecRequireType();
					
					TypeResolver typeResolver(context);
					const auto semType = typeResolver.resolveType(typeSpecType);
					const auto semRequireType = typeResolver.resolveType(typeSpecRequireType);
					
					return SEM::Predicate::Satisfies(semType, semRequireType);
				}
				case AST::PredicateDecl::SYMBOL: {
					const auto& astSymbolNode = astPredicateNode->symbol();
					const Name name = astSymbolNode->createName();
					
					const auto searchResult = performSearch(context, name);
					const auto templateVarMap = GenerateSymbolTemplateVarMap(context, astSymbolNode);
					
					if (searchResult.isAlias()) {
						auto& alias = searchResult.alias();
						(void) context.aliasTypeResolver().resolveAliasType(alias);
						
						const auto aliasValue = alias.value().substitute(templateVarMap);
						if (!aliasValue.type()->isBuiltInBool()) {
							context.issueDiag(PredicateAliasNotBoolDiag(name, aliasValue.type()),
							                  location);
							return SEM::Predicate::False();
						}
						
						return aliasValue.makePredicate();
					} else if (searchResult.isTemplateVar()) {
						auto& templateVar = searchResult.templateVar();
						
						if (!templateVar.type()->isBuiltInBool()) {
							context.issueDiag(PredicateTemplateVarNotBoolDiag(name, templateVar.type()),
							                  location);
							return SEM::Predicate::False();
						}
						
						return SEM::Predicate::Variable(&templateVar);
					} else if (!searchResult.isNone()) {
						context.issueDiag(InvalidSymbolInPredicateDiag(name), location);
						return SEM::Predicate::False();
					} else {
						context.issueDiag(UnknownSymbolInPredicateDiag(name), location);
						return SEM::Predicate::False();
					}
				}
				case AST::PredicateDecl::AND: {
					auto leftExpr = ConvertPredicate(context, astPredicateNode->andLeft());
					auto rightExpr = ConvertPredicate(context, astPredicateNode->andRight());
					return SEM::Predicate::And(std::move(leftExpr), std::move(rightExpr));
				}
				case AST::PredicateDecl::OR: {
					auto leftExpr = ConvertPredicate(context, astPredicateNode->orLeft());
					auto rightExpr = ConvertPredicate(context, astPredicateNode->orRight());
					return SEM::Predicate::Or(std::move(leftExpr), std::move(rightExpr));
				}
			}
			
			locic_unreachable("Unknown AST Predicate kind.");
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
			
			locic_unreachable("Unknown AST ConstSpecifier kind.");
		}
		
		SEM::Predicate ConvertPredicateSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode,
				const bool noneValue, const bool noPredicateValue) {
			if (astRequireSpecifierNode.isNull()) {
				return SEM::Predicate::FromBool(noneValue);
			}
			
			switch (astRequireSpecifierNode->kind()) {
				case AST::RequireSpecifier::NONE:
				{
					return SEM::Predicate::FromBool(noneValue);
				}
				case AST::RequireSpecifier::NOPREDICATE:
				{
					return SEM::Predicate::FromBool(noPredicateValue);
				}
				case AST::RequireSpecifier::EXPR:
				{
					return ConvertPredicate(context, astRequireSpecifierNode->expr());
				}
			}
			
			locic_unreachable("Unknown AST RequireSpecifier kind.");
		}
		
		class PushAssumedSatisfies {
		public:
			PushAssumedSatisfies(Context& context, const AST::Type* const checkType, const AST::Type* const requireType)
			: context_(context) {
				context_.pushAssumeSatisfies(checkType, requireType);
			}
			
			~PushAssumedSatisfies() {
				context_.popAssumeSatisfies();
			}
			
		private:
			Context& context_;
			
		};
		
		class PredicateHasLiteralFalseDiag: public Error {
		public:
			PredicateHasLiteralFalseDiag() { }
			
			std::string toString() const {
				return "predicate has literal 'false'";
			}
			
		};
		
		class PredicateVariableNotFoundDiag: public Error {
		public:
			PredicateVariableNotFoundDiag(const String name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("predicate variable '%s' not found",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		OptionalDiag
		evaluatePredicate(Context& context, const SEM::Predicate& predicate, const AST::TemplateVarMap& variableAssignments) {
			switch (predicate.kind()) {
				case SEM::Predicate::TRUE:
				{
					return OptionalDiag();
				}
				case SEM::Predicate::FALSE:
				{
					return OptionalDiag(PredicateHasLiteralFalseDiag());
				}
				case SEM::Predicate::AND:
				{
					auto leftResult = evaluatePredicate(context, predicate.andLeft(), variableAssignments);
					if (!leftResult) {
						return leftResult;
					}
					
					return evaluatePredicate(context, predicate.andRight(), variableAssignments);
				}
				case SEM::Predicate::OR:
				{
					auto leftResult = evaluatePredicate(context, predicate.orLeft(), variableAssignments);
					if (leftResult) {
						return leftResult;
					}
					
					auto rightResult = evaluatePredicate(context, predicate.orRight(), variableAssignments);
					if (rightResult) {
						return rightResult;
					}
					
					return leftResult;
				}
				case SEM::Predicate::SATISFIES:
				{
					const auto checkType = predicate.satisfiesType();
					const auto requireType = predicate.satisfiesRequirement();
					
					// Some of the requirements can depend on the template values provided.
					const auto substitutedCheckType = checkType->substitute(variableAssignments);
					const auto substitutedRequireType = requireType->substitute(variableAssignments);
					
					if (substitutedCheckType->isAuto()) {
						// Presumably this will work.
						// TODO: fix this by removing auto type!
						return OptionalDiag();
					}
					
					// Avoid cycles such as:
					// 
					// template <typename T : SomeType<T>>
					// interface SomeType { }
					// 
					// This is done by first checking if T : SomeType<T>,
					// which itself will check that T : SomeType<T> (since T
					// is an argument to 'SomeType') but on the second (nested)
					// check simply assuming that the result is true (since
					// this is being used to compute whether it is itself true
					// and a cyclic dependency like this is acceptable).
					if (context.isAssumedSatisfies(substitutedCheckType, substitutedRequireType)) {
						return OptionalDiag();
					}
					
					PushAssumedSatisfies assumedSatisfies(context, substitutedCheckType, substitutedRequireType);
					
					const auto sourceMethodSet = getTypeMethodSet(context, substitutedCheckType);
					const auto requireMethodSet = getTypeMethodSet(context, substitutedRequireType);
					
					return methodSetSatisfiesRequirement(context, sourceMethodSet, requireMethodSet);
				}
				case SEM::Predicate::VARIABLE:
				{
					const auto templateVar = predicate.variableTemplateVar();
					const auto iterator = variableAssignments.find(templateVar);
					
					if (iterator == variableAssignments.end()) {
						// TODO: we should be looking at the function/type's require()
						// predicate here.
						return OptionalDiag(PredicateVariableNotFoundDiag(templateVar->fullName().last()));
					}
					
					const auto& templateValue = iterator->second;
					return evaluatePredicate(context, templateValue.makePredicate(), variableAssignments);
				}
			}
			
			locic_unreachable("Unknown predicate kind.");
		}
		
		bool doesPredicateImplyPredicate(Context& /*context*/, const SEM::Predicate& firstPredicate, const SEM::Predicate& secondPredicate) {
			// TODO: actually prove in the general case that one implies the other.
			
			if (firstPredicate == secondPredicate) {
				// Equivalent predicates imply each other.
				return true;
			} else if (firstPredicate.isFalse() || secondPredicate.isTrue()) {
				// F => anything
				// anything => T
				return true;
			} else {
				return false;
			}
		}
		
		SEM::Predicate reducePredicate(Context& context, SEM::Predicate predicate) {
			switch (predicate.kind()) {
				case SEM::Predicate::TRUE:
				case SEM::Predicate::FALSE:
				{
					return predicate;
				}
				case SEM::Predicate::AND:
				{
					auto left = reducePredicate(context, predicate.andLeft().copy());
					auto right = reducePredicate(context, predicate.andRight().copy());
					return SEM::Predicate::And(std::move(left), std::move(right));
				}
				case SEM::Predicate::OR:
				{
					auto left = reducePredicate(context, predicate.orLeft().copy());
					auto right = reducePredicate(context, predicate.orRight().copy());
					return SEM::Predicate::Or(std::move(left), std::move(right));
				}
				case SEM::Predicate::SATISFIES:
				{
					const auto checkType = predicate.satisfiesType();
					const auto requireType = predicate.satisfiesRequirement();
					
					// Avoid cycles such as:
					// 
					// template <typename T : SomeType<T>>
					// interface SomeType { }
					// 
					// This is done by first checking if T : SomeType<T>,
					// which itself will check that T : SomeType<T> (since T
					// is an argument to 'SomeType') but on the second (nested)
					// check simply assuming that the result is true (since
					// this is being used to compute whether it is itself true
					// and a cyclic dependency like this is acceptable).
					if (context.isAssumedSatisfies(checkType, requireType)) {
						return SEM::Predicate::True();
					}
					
					PushAssumedSatisfies assumedSatisfies(context, checkType, requireType);
					
					if (checkType->isAuto()) {
						// Presumably this is OK...
						// TODO: remove auto from here.
						return SEM::Predicate::True();
					}
					
					const auto sourceMethodSet = getTypeMethodSet(context, checkType);
					const auto requireMethodSet = getTypeMethodSet(context, requireType);
					
					const bool result = methodSetSatisfiesRequirement(context, sourceMethodSet, requireMethodSet);
					
					if (result) {
						// If the result is true then we
						// know for sure that the check
						// type satisfies the requirement,
						// but a false result might just
						// be a lack of information.
						return SEM::Predicate::True();
					}
					
					if (!checkType->dependsOnOnly({}) || !requireType->dependsOnOnly({})) {
						// Types still depend on some template variables, so can't reduce.
						return predicate;
					}
					
					return SEM::Predicate::False();
				}
				case SEM::Predicate::VARIABLE:
				{
					return predicate;
				}
			}
			
			locic_unreachable("Unknown predicate kind.");
		}
		
	}
	
}


