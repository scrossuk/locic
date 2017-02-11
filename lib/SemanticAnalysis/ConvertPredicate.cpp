#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Debug.hpp>
#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/SatisfyChecker.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>
#include <locic/SemanticAnalysis/Unifier.hpp>
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
		
		AST::Predicate ConvertPredicate(Context& context, const AST::Node<AST::PredicateDecl>& astPredicateNode) {
			const auto& location = astPredicateNode.location();
			
			switch (astPredicateNode->kind()) {
				case AST::PredicateDecl::TRUE: {
					return AST::Predicate::True();
				}
				case AST::PredicateDecl::FALSE: {
					return AST::Predicate::False();
				}
				case AST::PredicateDecl::SELFCONST: {
					return AST::Predicate::SelfConst();
				}
				case AST::PredicateDecl::BRACKET: {
					return ConvertPredicate(context, astPredicateNode->bracketExpr());
				}
				case AST::PredicateDecl::TYPESPEC: {
					auto& typeSpecType = astPredicateNode->typeSpecType();
					auto& typeSpecRequireType = astPredicateNode->typeSpecRequireType();
					
					TypeResolver typeResolver(context);
					const auto astType = typeResolver.resolveType(typeSpecType);
					const auto astRequireType = typeResolver.resolveType(typeSpecRequireType);
					
					return AST::Predicate::Satisfies(astType, astRequireType);
				}
				case AST::PredicateDecl::SYMBOL: {
					const auto& astSymbolNode = astPredicateNode->symbol();
					const Name name = astSymbolNode->createName();
					
					const auto searchResult = performSearch(context, name);
					const auto templateVarMap = GenerateSymbolTemplateVarMap(context, astSymbolNode);
					
					if (searchResult.isAlias()) {
						auto& alias = searchResult.alias();
						(void) context.aliasTypeResolver().resolveAliasType(alias);
						
						const auto aliasValue = alias.value().substitute(templateVarMap,
						                                                 /*selfconst=*/AST::Predicate::SelfConst());
						if (!aliasValue.type()->isBuiltInBool()) {
							context.issueDiag(PredicateAliasNotBoolDiag(name, aliasValue.type()),
							                  location);
							return AST::Predicate::False();
						}
						
						return aliasValue.makePredicate();
					} else if (searchResult.isTemplateVar()) {
						auto& templateVar = searchResult.templateVar();
						
						if (!templateVar.type()->isBuiltInBool()) {
							context.issueDiag(PredicateTemplateVarNotBoolDiag(name, templateVar.type()),
							                  location);
							return AST::Predicate::False();
						}
						
						return AST::Predicate::Variable(&templateVar);
					} else if (!searchResult.isNone()) {
						context.issueDiag(InvalidSymbolInPredicateDiag(name), location);
						return AST::Predicate::False();
					} else {
						context.issueDiag(UnknownSymbolInPredicateDiag(name), location);
						return AST::Predicate::False();
					}
				}
				case AST::PredicateDecl::AND: {
					auto leftExpr = ConvertPredicate(context, astPredicateNode->andLeft());
					auto rightExpr = ConvertPredicate(context, astPredicateNode->andRight());
					return AST::Predicate::And(std::move(leftExpr), std::move(rightExpr));
				}
				case AST::PredicateDecl::OR: {
					auto leftExpr = ConvertPredicate(context, astPredicateNode->orLeft());
					auto rightExpr = ConvertPredicate(context, astPredicateNode->orRight());
					return AST::Predicate::Or(std::move(leftExpr), std::move(rightExpr));
				}
			}
			
			locic_unreachable("Unknown AST Predicate kind.");
		}
		
		AST::Predicate ConvertConstSpecifier(Context& context, const AST::Node<AST::ConstSpecifier>& astConstSpecifierNode) {
			switch (astConstSpecifierNode->kind()) {
				case AST::ConstSpecifier::NONE:
					// No specifier means it's false (i.e. always not const).
					return AST::Predicate::False();
				case AST::ConstSpecifier::CONST:
					// 'const' means it's true (i.e. always const).
					return AST::Predicate::True();
				case AST::ConstSpecifier::MUTABLE:
					// 'mutable' means it's false (i.e. always not const).
					return AST::Predicate::False();
				case AST::ConstSpecifier::EXPR:
				{
					return ConvertPredicate(context, astConstSpecifierNode->predicate());
				}
			}
			
			locic_unreachable("Unknown AST ConstSpecifier kind.");
		}
		
		AST::Predicate ConvertPredicateSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode,
				const bool noneValue, const bool noPredicateValue) {
			if (astRequireSpecifierNode.isNull()) {
				return AST::Predicate::FromBool(noneValue);
			}
			
			switch (astRequireSpecifierNode->kind()) {
				case AST::RequireSpecifier::NONE:
				{
					return AST::Predicate::FromBool(noneValue);
				}
				case AST::RequireSpecifier::NOPREDICATE:
				{
					return AST::Predicate::FromBool(noPredicateValue);
				}
				case AST::RequireSpecifier::EXPR:
				{
					return ConvertPredicate(context, astRequireSpecifierNode->expr());
				}
			}
			
			locic_unreachable("Unknown AST RequireSpecifier kind.");
		}
		
		class PredicateHasLiteralFalseDiag: public Error {
		public:
			PredicateHasLiteralFalseDiag() { }
			
			std::string toString() const {
				return "predicate has literal 'false'";
			}
			
		};
		
		class PredicateHasLiteralSelfConstDiag: public Error {
		public:
			PredicateHasLiteralSelfConstDiag() { }
			
			std::string toString() const {
				return "predicate has literal 'selfconst'";
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
		evaluatePredicate(Context& context, const AST::Predicate& predicate, const AST::TemplateVarMap& variableAssignments) {
			switch (predicate.kind()) {
				case AST::Predicate::TRUE:
				{
					return SUCCESS;
				}
				case AST::Predicate::FALSE:
				{
					return PredicateHasLiteralFalseDiag();
				}
				case AST::Predicate::SELFCONST:
				{
					return PredicateHasLiteralSelfConstDiag();
				}
				case AST::Predicate::AND:
				{
					auto leftResult = evaluatePredicate(context, predicate.andLeft(), variableAssignments);
					if (leftResult.failed()) {
						return leftResult;
					}
					
					return evaluatePredicate(context, predicate.andRight(), variableAssignments);
				}
				case AST::Predicate::OR:
				{
					auto leftResult = evaluatePredicate(context, predicate.orLeft(), variableAssignments);
					if (leftResult.success()) {
						return leftResult;
					}
					
					auto rightResult = evaluatePredicate(context, predicate.orRight(), variableAssignments);
					if (rightResult.success()) {
						return rightResult;
					}
					
					return leftResult;
				}
				case AST::Predicate::SATISFIES:
				{
					const auto checkType = predicate.satisfiesType();
					const auto requireType = predicate.satisfiesRequirement();
					
					// Some of the requirements can depend on the template values provided.
					const auto substitutedCheckType = checkType->substitute(variableAssignments,
					                                                        /*selfconst=*/AST::Predicate::SelfConst());
					const auto substitutedRequireType = requireType->substitute(variableAssignments,
					                                                            /*selfconst=*/AST::Predicate::SelfConst());
					
					if (substitutedCheckType->isAuto()) {
						// Presumably this will work.
						// TODO: fix this by removing auto type!
						return SUCCESS;
					}
					
					Unifier unifier;
					return SatisfyChecker(context, unifier).satisfies(substitutedCheckType,
					                                                  substitutedRequireType);
				}
				case AST::Predicate::VARIABLE:
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
		
		AST::Predicate reducePredicate(Context& context, AST::Predicate predicate) {
			Unifier unifier;
			return SatisfyChecker(context, unifier).reducePredicate(std::move(predicate));
		}
		
	}
	
}


