#include <stdio.h>

#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/Template.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void addRequireTypeInstance(Context& context, SEM::TemplateRequireMap& requireMap, SEM::TemplateVar* templateVar) {
			const auto name = templateVar->name() + "#spectype";
			const auto moduleScope = SEM::ModuleScope::Internal();
			std::unique_ptr<SEM::TypeInstance> typeInstance(new SEM::TypeInstance(context.semContext(), name, SEM::TypeInstance::TEMPLATETYPE, moduleScope));
			requireMap.insert(std::make_pair(templateVar, typeInstance.release()));
		}
		
		void addTypeToRequirement(Context& context, SEM::TypeInstance* const requireInstance, const SEM::Type* const newType) {
			assert(newType->isObjectOrTemplateVar());
			
			const auto typeInstance = getObjectOrSpecType(context, newType);
			const auto templateVarMap = newType->generateTemplateVarMap();
			
			for (const auto& newFunctionPair: typeInstance->functions()) {
				const auto& functionName = newFunctionPair.first;
				const auto& newFunction = newFunctionPair.second;
				
				// TODO: also skip unsatisfied requirement specifiers.
				if (newType->isConst() && !newFunction->isConstMethod()) {
					// Skip function.
					continue;
				}
				
				const auto& iterator = requireInstance->functions().find(functionName);
				if (iterator != requireInstance->functions().end()) {
					// Not a new method; must be merged with existing method.
					const auto& existingFunction = iterator->second;
					
					// TODO!
					(void) existingFunction;
					
					throw std::runtime_error("Merging functions not supported!");
				} else {
					// Entirely new method.
					const auto addFunction = newFunction->createTemplatedDecl()->fullSubstitute(requireInstance->name() + functionName, templateVarMap);
					requireInstance->functions().insert(std::make_pair(functionName, addFunction));
				}
			}
		}
		
		const SEM::TypeInstance* getObjectOrSpecType(Context& context, const SEM::Type* type) {
			assert(type->isObject() || type->isTemplateVar());
			return type->isObject() ?
				type->getObjectType() :
				getSpecType(context.scopeStack(), type->getTemplateVar());
		}
		
		TemplatedTypeInstance getTemplateTypeInstance(Context& context, const SEM::Type* type) {
			return std::make_pair(getObjectOrSpecType(context, type), type->generateTemplateVarMap());
		}
		
		SEM::TemplateVarMap GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbol) {
			const auto& location = astSymbol.location();
			
			const Name fullName = astSymbol->createName();
			assert(fullName.size() == astSymbol->size());
			
			SEM::TemplateVarMap templateVarMap;
			
			for (size_t i = 0; i < astSymbol->size(); i++) {
				const auto& astSymbolElement = astSymbol->at(i);
				const auto& astTemplateArgs = astSymbolElement->templateArguments();
				const size_t numTemplateArguments = astTemplateArgs->size();
				
				const Name name = fullName.substr(i + 1);
				
				const auto searchResult = performSearch(context, name);
				
				if (searchResult.isFunction() || searchResult.isTypeAlias() || searchResult.isTypeInstance()) {
					const auto& templateVariables =
						searchResult.isFunction() ?
							searchResult.function()->templateVariables() :
							searchResult.isTypeAlias() ?
								searchResult.typeAlias()->templateVariables() :
								searchResult.typeInstance()->templateVariables();
					
					const auto& typeRequirements =
						searchResult.isFunction() ?
							searchResult.function()->typeRequirements() :
							searchResult.isTypeAlias() ?
								searchResult.typeAlias()->typeRequirements() :
								searchResult.typeInstance()->typeRequirements();
					
					if (templateVariables.size() != numTemplateArguments) {
						throw ErrorException(makeString("Incorrect number of template "
							"arguments provided for function or type '%s'; %llu were required, "
							"but %llu were provided at position %s.",
							name.toString().c_str(),
							(unsigned long long) templateVariables.size(),
							(unsigned long long) numTemplateArguments,
							location.toString().c_str()));
					}
					
					// First generate template var -> type map.
					for (size_t j = 0; j < templateVariables.size(); j++) {
						const auto templateTypeValue = ConvertType(context, astTemplateArgs->at(j));
						templateVarMap.insert(std::make_pair(templateVariables.at(j), templateTypeValue));
					}
					
					// Then check all the types are valid parameters.
					for (size_t j = 0; j < templateVariables.size(); j++) {
						const auto templateTypeValue = ConvertType(context, astTemplateArgs->at(j))->resolveAliases();
						
						if (templateTypeValue->isAuto()) {
							// Presumably auto will always work...
							continue;
						}
						
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
						
						const auto templateVariable = templateVariables.at(j);
						const auto specTypeInstance = typeRequirements.at(templateVariable);
						
						const auto sourceType = getTemplateTypeInstance(context, templateTypeValue);
						const auto requireType = std::make_pair(specTypeInstance, templateVarMap);
						
						if (context.templateRequirementsComplete()) {
							if (!TemplateValueSatisfiesRequirement(sourceType, requireType)) {
								throw ErrorException(makeString("Type does not satisfy "
									"constraint for template parameter '%s' of function or type '%s' at position %s.",
									templateVariable->name().toString().c_str(),
									name.toString().c_str(),
									location.toString().c_str()));
							}
						} else {
							// Record this instantiation to be checked later.
							context.templateInstantiations().push_back(std::make_tuple(templateVariable, sourceType, requireType, name, location));
						}
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
			
			return templateVarMap;
		}
		
		std::vector<const SEM::Type*> GetTemplateValues(const SEM::TemplateVarMap& templateVarMap, const std::vector<SEM::TemplateVar*>& templateVariables) {
			std::vector<const SEM::Type*> templateArguments;
			templateArguments.reserve(templateVariables.size());
			for (const auto templateVar: templateVariables) {
				templateArguments.push_back(templateVarMap.at(templateVar));
			}
			return templateArguments;
		}
		
		static bool methodNamesMatch(const std::string& first, const std::string& second) {
			return CanonicalizeMethodName(first) == CanonicalizeMethodName(second);
		}
		
		static bool functionTypesCompatible(const SEM::Type* sourceType, const SEM::Type* destType) {
			assert(sourceType->isFunction());
			assert(destType->isFunction());
			
			if (sourceType == destType) {
				return true;
			}
			
			assert(!sourceType->isLval());
			assert(!destType->isLval());
			assert(!sourceType->isRef());
			assert(!destType->isRef());
			assert(!sourceType->isFunctionVarArg());
			assert(!destType->isFunctionVarArg());
			
			const auto& firstList = sourceType->getFunctionParameterTypes();
			const auto& secondList = destType->getFunctionParameterTypes();
			
			if (firstList.size() != secondList.size()) {
				return firstList.size() < secondList.size();
			}
			
			for (size_t i = 0; i < firstList.size(); i++) {
				if (firstList.at(i) != secondList.at(i)) {
					return false;
				}
			}
			
			const auto castReturnType =
				ImplicitCastTypeFormatOnly(
					sourceType->getFunctionReturnType(),
					destType->getFunctionReturnType(),
					Debug::SourceLocation::Null());
			if (castReturnType == nullptr) {
				return false;
			}
			
			if (sourceType->isFunctionMethod() != destType->isFunctionMethod()) {
				return false;
			}
			
			if (!sourceType->isFunctionNoExcept() && destType->isFunctionNoExcept()) {
				// Can't add 'noexcept' specifier.
				return false;
			}
			
			return true;
		}
		
		bool TemplateValueSatisfiesRequirement(const TemplatedTypeInstance& objectType, const TemplatedTypeInstance& requireType) {
			const auto objectInstance = objectType.first;
			const auto& objectTemplateVarMap = objectType.second;
			
			const auto requireInstance = requireType.first;
			const auto& requireTemplateVarMap = requireType.second;
			
			auto objectIterator = objectInstance->functions().begin();
			auto requireIterator = requireInstance->functions().begin();
			
			for (; requireIterator != requireInstance->functions().end(); ++objectIterator) {
				const auto requireFunction = requireIterator->second;
				
				if (objectIterator == objectInstance->functions().end()) {
					// If all the object methods have been considered, but
					// there's still an required method to consider, then
					// that method must NOT be present in the object type.
					printf("\n\nMethod not found:\n\n%s\n\n",
						requireFunction->name().toString().c_str());
					return false;
				}
				
				const auto objectFunction = objectIterator->second;
				
				if (!methodNamesMatch(objectFunction->name().last(), requireFunction->name().last())) {
					continue;
				}
				
				// Can't cast mutator method to const method.
				if (!objectFunction->isConstMethod() && requireFunction->isConstMethod()) {
					printf("\n\nNot const-compatible:\n\n%s\n\n%s\n\n",
						objectFunction->name().toString().c_str(),
						requireFunction->name().toString().c_str());
					return false;
				}
				
				// Substitute any template variables in the function types.
				const auto objectFunctionType = objectFunction->type()->substitute(objectTemplateVarMap);
				const auto requireFunctionType = requireFunction->type()->substitute(requireTemplateVarMap);
				
				// Function types must be equivalent.
				if (!functionTypesCompatible(objectFunctionType, requireFunctionType)) {
					printf("\n\nNot compatible:\n\n%s\n\n%s\n\n",
						objectFunctionType->toString().c_str(),
						requireFunctionType->toString().c_str());
					return false;
				}
				
				++requireIterator;
			}
			
			return true;
		}
		
	}
	
}

