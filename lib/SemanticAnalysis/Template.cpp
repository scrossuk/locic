#include <stdio.h>

#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/Frontend/OptionalDiag.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TemplateInst.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			AST::TemplatedObject& getTemplatedObject(const SearchResult& searchResult) {
				switch (searchResult.kind()) {
					case SearchResult::ALIAS:
						return searchResult.alias();
					case SearchResult::FUNCTION:
						return searchResult.function();
					case SearchResult::TYPEINSTANCE:
						return searchResult.typeInstance();
					default:
						locic_unreachable("Unknown templated object search result.");
				}
			}
			
		}
		
		class TemplateArgsDoNotSatisfyRequirePredicateDiag : public Error {
		public:
			TemplateArgsDoNotSatisfyRequirePredicateDiag(const SEM::Predicate& requirePredicate,
			                                             const Name& name)
			: requirePredicateString_(requirePredicate.toString()), name_(name.copy()) { }

			std::string toString() const {
				return makeString("template arguments do not satisfy require predicate "
				                  "'%s' of function or type '%s'", requirePredicateString_.c_str(),
				                  name_.toString(/*addPrefix=*/false).c_str());
			}

		private:
			std::string requirePredicateString_;
			Name name_;
			
		};
		
		class TemplateArgHasInvalidTypeDiag : public Error {
		public:
			TemplateArgHasInvalidTypeDiag(const String& name, const SEM::Type* expectedType,
			                              const SEM::Type* actualType)
			: name_(name), expectedTypeString_(expectedType->toDiagString()),
			actualTypeString_(actualType->toDiagString()) { }

			std::string toString() const {
				return makeString("template argument has type '%s', which doesn't match "
				                  "type '%s' of template variable '%s'", actualTypeString_.c_str(),
				                  expectedTypeString_.c_str(), name_.c_str());
			}

		private:
			String name_;
			std::string expectedTypeString_;
			std::string actualTypeString_;
			
		};
		
		void CheckTemplateInstantiation(Context& context,
		                                const AST::TemplatedObject& templatedObject,
		                                const AST::TemplateVarMap& variableAssignments,
		                                const Debug::SourceLocation& location) {
			// Requires predicate is already known so check it immediately.
			const auto& requiresPredicate = templatedObject.requiresPredicate();
			
			auto result = evaluatePredicate(context, requiresPredicate, variableAssignments);
			if (!result) {
				const auto substitutedRequirePredicate = requiresPredicate.substitute(variableAssignments);
				context.issueDiag(TemplateArgsDoNotSatisfyRequirePredicateDiag(substitutedRequirePredicate,
				                                                               templatedObject.fullName()),
				                  location, std::move(result));
			}
			
			for (const auto& assignment: variableAssignments) {
				const auto& templateVar = assignment.first;
				const auto& templateValue = assignment.second;
				const auto templateVarType = templateVar->type()->substitute(variableAssignments)->resolveAliases();
				const auto templateValueType = templateValue.type()->resolveAliases();
				
				if (templateVarType != templateValueType) {
					context.issueDiag(TemplateArgHasInvalidTypeDiag(templateVar->fullName().last(),
					                                                templateVarType,
					                                                templateValueType),
					                  location);
				}
				
				if (templateValue.isTypeRef()) {
					const auto templateTypeValue = templateValue.typeRefType()->resolveAliases();
					
					// Presumably auto will always work...
					if (!templateTypeValue->isAuto()) {
						assert(templateTypeValue->isObjectOrTemplateVar());
						assert(!templateTypeValue->isInterface());
					}
				}
			}
		}
		
		AST::TemplateVarMap GenerateTemplateVarMap(Context& context, const AST::TemplatedObject& templatedObject,
				SEM::ValueArray values, const Debug::SourceLocation& location, AST::TemplateVarMap variableAssignments) {
			const auto& templateVariables = templatedObject.templateVariables();
			
			for (size_t i = 0; i < std::min(templateVariables.size(), values.size()); i++) {
				const auto& templateVar = templateVariables[i];
				auto& templateValue = values[i];
				variableAssignments.insert(std::make_pair(templateVar, std::move(templateValue)));
			}
			
			for (auto& assignment: variableAssignments) {
				const auto& templateVar = assignment.first;
				auto& templateValue = assignment.second;
				if (!templateValue.isConstant()) {
					continue;
				}
				
				// This is a temporary mechanism by which a template
				// value constant is re-typed as the destination type,
				// since performing a cast here leads to the compiler
				// trying to generate method sets before it has all
				// the relevant information.
				//
				// In future, this code needs to perform an actual
				// cast and the issue mentioned above needs to be
				// resolved.
				const auto templateVarType = templateVar->type()->substitute(variableAssignments);
				templateValue = SEM::Value::Constant(templateValue.constant(),
				                                     templateVarType);
			}
			
			// Check the assignments satisfy the requires predicate.
			// 
			// It's possible that we get to this point before the requires predicate
			// is actually known, so we have to save the types provided and a pointer
			// to the templated object (e.g. a type instance) so the requires predicate
			// can be queried from it later.
			// 
			// This is then checked as part of a Semantic Analysis pass that runs when
			// all the requires predicates are guaranteed to be known.
			if (context.templateRequirementsComplete()) {
				CheckTemplateInstantiation(context,
				                           templatedObject,
				                           variableAssignments,
				                           location);
				
			} else {
				// Record this instantiation to be checked later.
				context.templateInstantiations().push_back(
					TemplateInst(context.scopeStack().copy(),
					             variableAssignments.copy(),
					             templatedObject,
					             location));
			}
			
			return variableAssignments;
		}
		
		class InvalidTemplateArgCountDiag: public Error {
		public:
			InvalidTemplateArgCountDiag(const Name& name, size_t argsExpected,
			                            size_t argsGiven)
			: name_(name.copy()), argsExpected_(argsExpected),
			argsGiven_(argsGiven) { }
			
			std::string toString() const {
				return makeString("incorrect number of template arguments provided "
				                  "for function or type '%s'; %zu were required, but %zu "
				                  "were provided", name_.toString(/*addPrefix=*/false).c_str(),
				                  argsExpected_, argsGiven_);
			}
			
		private:
			Name name_;
			size_t argsExpected_;
			size_t argsGiven_;
			
		};
		
		class UnexpectedTemplateArgDiag: public Error {
		public:
			UnexpectedTemplateArgDiag(const Name& name, size_t argsGiven)
			: name_(name.copy()), argsGiven_(argsGiven) { }
			
			std::string toString() const {
				return makeString("%zu template arguments provided for non-function "
				                  "and non-type node '%s'; none should be provided",
				                  argsGiven_, name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			size_t argsGiven_;
			
		};
		
		AST::TemplateVarMap GenerateSymbolTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbol) {
			const auto& location = astSymbol.location();
			
			const Name fullName = astSymbol->createName();
			assert(fullName.size() == astSymbol->size());
			
			AST::TemplateVarMap variableAssignments;
			
			for (size_t i = 0; i < astSymbol->size(); i++) {
				const auto& astSymbolElement = astSymbol->at(i);
				const auto& astTemplateArgs = astSymbolElement->templateArguments();
				
				const Name name = fullName.substr(i + 1);
				
				const auto searchResult = performSearch(context, name);
				
				if (searchResult.isFunction() || searchResult.isAlias() || searchResult.isTypeInstance()) {
					const auto& templatedObject = getTemplatedObject(searchResult);
					const auto& templateVariables = templatedObject.templateVariables();
					
					SEM::ValueArray templateValues;
					for (const auto& astTemplateArg: *astTemplateArgs) {
						templateValues.push_back(ConvertValue(context, astTemplateArg));
					}
					
					if (templateValues.size() != templateVariables.size()) {
						context.issueDiag(InvalidTemplateArgCountDiag(name,
						                                              templateVariables.size(),
						                                              templateValues.size()),
						                  location);
						
						while (templateValues.size() < templateVariables.size()) {
							templateValues.push_back(templateVariables[templateValues.size()]->selfRefValue());
						}
					}
					
					variableAssignments = GenerateTemplateVarMap(context, templatedObject,
					                                             std::move(templateValues),
					                                             location, std::move(variableAssignments));
				} else if (astTemplateArgs->size() > 0) {
					context.issueDiag(UnexpectedTemplateArgDiag(name, astTemplateArgs->size()),
					                  location);
				}
			}
			
			return variableAssignments;
		}
		
		SEM::ValueArray GetTemplateValues(const AST::TemplateVarMap& templateVarMap, const AST::TemplateVarArray& templateVariables) {
			SEM::ValueArray templateArguments;
			templateArguments.reserve(templateVariables.size());
			for (const auto templateVar: templateVariables) {
				templateArguments.push_back(templateVarMap.at(templateVar).copy());
			}
			return templateArguments;
		}
		
		SEM::ValueArray makeTemplateArgs(Context& context, SEM::TypeArray typeArray) {
			SEM::ValueArray templateArguments;
			templateArguments.reserve(typeArray.size());
			
			const auto typenameType = getBuiltInType(context, context.getCString("typename_t"), {});
			
			for (const auto& arg: typeArray) {
				templateArguments.push_back(SEM::Value::TypeRef(arg, typenameType->createStaticRefType(arg)));
			}
			
			return templateArguments;
		}
		
	}
	
}

