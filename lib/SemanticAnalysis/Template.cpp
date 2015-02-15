#include <stdio.h>

#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/Template.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			SEM::TemplatedObject* getTemplatedObject(const SearchResult& searchResult) {
				switch (searchResult.kind()) {
					case SearchResult::FUNCTION:
						return searchResult.function();
					case SearchResult::TYPEALIAS:
						return searchResult.typeAlias();
					case SearchResult::TYPEINSTANCE:
						return searchResult.typeInstance();
					default:
						throw std::logic_error("Unknown templated object search result.");
				}
			}
			
		}
		
		SEM::TemplateVarMap GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbol) {
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
				
				if (searchResult.isFunction() || searchResult.isTypeAlias() || searchResult.isTypeInstance()) {
					const auto templatedObject = getTemplatedObject(searchResult);
					const auto& templateVariables = templatedObject->templateVariables();
					
					if (templateVariables.size() != numTemplateArguments) {
						throw ErrorException(makeString("Incorrect number of template "
							"arguments provided for function or type '%s'; %llu were required, "
							"but %llu were provided at position %s.",
							name.toString().c_str(),
							(unsigned long long) templateVariables.size(),
							(unsigned long long) numTemplateArguments,
							location.toString().c_str()));
					}
					
					// Add template var -> type assignments to map.
					for (size_t j = 0; j < templateVariables.size(); j++) {
						const auto templateTypeValue = ConvertType(context, astTemplateArgs->at(j))->resolveAliases();
						
						// Presumably auto will always work...
						if (!templateTypeValue->isAuto()) {
							if (!templateTypeValue->isObjectOrTemplateVar()) {
								throw ErrorException(makeString("Cannot use non-object and non-template type '%s' "
									"as template parameter %llu for function or type '%s' at position %s.",
									templateTypeValue->toString().c_str(),
									(unsigned long long) j,
									name.toString().c_str(),
									location.toString().c_str()));
							}
							
							if (templateTypeValue->isInterface()) {
								throw ErrorException(makeString("Cannot use abstract type '%s' "
									"as template parameter %llu for function or type '%s' at position %s.",
									templateTypeValue->getObjectType()->name().toString().c_str(),
									(unsigned long long) j,
									name.toString().c_str(),
									location.toString().c_str()));
							}
						}
						
						variableAssignments.insert(std::make_pair(templateVariables.at(j), templateTypeValue));
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
						// Requires predicate is already known so check it immediately.
						const auto& requiresPredicate = templatedObject->requiresPredicate();
						
						if (!evaluatePredicate(context, requiresPredicate, variableAssignments)) {
							throw ErrorException(makeString("Template arguments do not satisfy "
								"requires predicate '%s' of function or type '%s' at position %s.",
								requiresPredicate.toString().c_str(),
								name.toString().c_str(),
								location.toString().c_str()));
						}
					} else {
						// Record this instantiation to be checked later.
						context.templateInstantiations().push_back(
							std::make_tuple(context.scopeStack(), variableAssignments, templatedObject, name, location));
					}
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
		
		SEM::TypeArray GetTemplateValues(const SEM::TemplateVarMap& templateVarMap, const std::vector<SEM::TemplateVar*>& templateVariables) {
			SEM::TypeArray templateArguments;
			templateArguments.reserve(templateVariables.size());
			for (const auto templateVar: templateVariables) {
				templateArguments.push_back(templateVarMap.at(templateVar));
			}
			return templateArguments;
		}
		
	}
	
}

