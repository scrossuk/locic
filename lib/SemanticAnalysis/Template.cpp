#include <stdio.h>

#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
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
			
			SEM::TemplatedObject& getTemplatedObject(const SearchResult& searchResult) {
				switch (searchResult.kind()) {
					case SearchResult::ALIAS:
						return searchResult.alias();
					case SearchResult::FUNCTION:
						return searchResult.function();
					case SearchResult::TYPEINSTANCE:
						return searchResult.typeInstance();
					default:
						throw std::logic_error("Unknown templated object search result.");
				}
			}
			
		}
		
		void CheckTemplateInstantiation(Context& context,
		                                const SEM::TemplatedObject& templatedObject,
		                                const SEM::TemplateVarMap& variableAssignments,
		                                const Debug::SourceLocation& location) {
			// Requires predicate is already known so check it immediately.
			const auto& requiresPredicate = templatedObject.requiresPredicate();
			
			// Conservatively assume require predicate is not satisified if result is undetermined.
			const bool satisfiesRequiresDefault = false;
			
			if (!evaluatePredicateWithDefault(context, requiresPredicate, variableAssignments, satisfiesRequiresDefault)) {
				throw ErrorException(makeString("Template arguments do not satisfy "
					"requires predicate '%s' of function or type '%s' at position %s.",
					requiresPredicate.substitute(variableAssignments).toString().c_str(),
					templatedObject.name().toString().c_str(),
					location.toString().c_str()));
			}
			
			for (const auto& assignment: variableAssignments) {
				const auto& templateVar = assignment.first;
				const auto& templateValue = assignment.second;
				const auto templateVarType = templateVar->type()->substitute(variableAssignments)->resolveAliases();
				const auto templateValueType = templateValue.type()->resolveAliases();
				
				if (templateVarType != templateValueType) {
					throw ErrorException(makeString("Template argument '%s' has type '%s', which doesn't match type '%s' of template variable '%s', at position %s.",
						templateValue.toString().c_str(),
						templateValueType->toString().c_str(),
						templateVarType->toString().c_str(),
						templateVar->name().toString().c_str(), location.toString().c_str()));
				}
				
				if (templateValue.isTypeRef()) {
					const auto templateTypeValue = templateValue.typeRefType()->resolveAliases();
					
					// Presumably auto will always work...
					if (!templateTypeValue->isAuto()) {
						if (!templateTypeValue->isObjectOrTemplateVar()) {
							throw ErrorException(makeString("Cannot use non-object and non-template type '%s' "
								"as template parameter %llu for function or type '%s' at position %s.",
								templateTypeValue->toString().c_str(),
								(unsigned long long) templateVar->index(),
								templatedObject.name().toString().c_str(),
								location.toString().c_str()));
						}
						
						if (templateTypeValue->isInterface()) {
							throw ErrorException(makeString("Cannot use abstract type '%s' "
								"as template parameter %llu for function or type '%s' at position %s.",
								templateTypeValue->getObjectType()->name().toString().c_str(),
								(unsigned long long) templateVar->index(),
								templatedObject.name().toString().c_str(),
								location.toString().c_str()));
						}
					}
				}
			}
		}
		
		SEM::TemplateVarMap GenerateTemplateVarMap(Context& context, const SEM::TemplatedObject& templatedObject,
				SEM::ValueArray values, const Debug::SourceLocation& location, SEM::TemplateVarMap variableAssignments) {
			const auto& templateVariables = templatedObject.templateVariables();
			
			assert(templateVariables.size() == values.size());
			
			for (size_t i = 0; i < templateVariables.size(); i++) {
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
		
		SEM::TemplateVarMap GenerateSymbolTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbol) {
			const auto& location = astSymbol.location();
			
			const Name fullName = astSymbol->createName();
			assert(fullName.size() == astSymbol->size());
			
			SEM::TemplateVarMap variableAssignments;
			
			for (size_t i = 0; i < astSymbol->size(); i++) {
				const auto& astSymbolElement = astSymbol->at(i);
				const auto& astTemplateArgs = astSymbolElement->templateArguments();
				const size_t numTemplateArguments = astTemplateArgs->size();
				
				const Name name = fullName.substr(i + 1);
				
				const auto searchResult = performSearch(context, name);
				
				if (searchResult.isFunction() || searchResult.isAlias() || searchResult.isTypeInstance()) {
					const auto& templatedObject = getTemplatedObject(searchResult);
					const auto& templateVariables = templatedObject.templateVariables();
					
					if (templateVariables.size() != numTemplateArguments) {
						throw ErrorException(makeString("Incorrect number of template "
							"arguments provided for function or type '%s'; %llu were required, "
							"but %llu were provided at position %s.",
							name.toString().c_str(),
							(unsigned long long) templateVariables.size(),
							(unsigned long long) numTemplateArguments,
							location.toString().c_str()));
					}
					
					SEM::ValueArray templateValues;
					for (const auto& astTemplateArg: *astTemplateArgs) {
						templateValues.push_back(ConvertValue(context, astTemplateArg));
					}
					
					variableAssignments = GenerateTemplateVarMap(context, templatedObject, std::move(templateValues), location, std::move(variableAssignments));
				} else {
					if (numTemplateArguments > 0) {
						throw ErrorException(makeString("%llu template "
							"arguments provided for non-function and non-type node '%s'; "
							"none should be provided at position %s.",
							(unsigned long long) numTemplateArguments,
							name.toString().c_str(),
							location.toString().c_str()));
					}
				}
			}
			
			return variableAssignments;
		}
		
		SEM::ValueArray GetTemplateValues(const SEM::TemplateVarMap& templateVarMap, const SEM::TemplateVarArray& templateVariables) {
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

