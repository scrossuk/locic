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

namespace locic {

	namespace SemanticAnalysis {
	
		Map<SEM::TemplateVar*, SEM::Type*> GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbol) {
			const auto& location = astSymbol.location();
			
			const Name fullName = astSymbol->createName();
			assert(fullName.size() == astSymbol->size());
			
			Map<SEM::TemplateVar*, SEM::Type*> templateVarMap;
			
			for (size_t i = 0; i < astSymbol->size(); i++) {
				const auto& astSymbolElement = astSymbol->at(i);
				const auto& astTemplateArgs = astSymbolElement->templateArguments();
				const size_t numTemplateArguments = astTemplateArgs->size();
				
				const Name name = fullName.substr(i + 1);
				
				const auto searchResult = performSearch(context, name);
				
				if (searchResult.isTypeInstance()) {
					const auto typeInstance = searchResult.typeInstance();
					
					const size_t numTemplateVariables = typeInstance->templateVariables().size();
					if (numTemplateVariables != numTemplateArguments) {
						throw ErrorException(makeString("Incorrect number of template "
							"arguments provided for type '%s'; %llu were required, "
							"but %llu were provided at position %s.",
							name.toString().c_str(),
							(unsigned long long) numTemplateVariables,
							(unsigned long long) numTemplateArguments,
							location.toString().c_str()));
					}
					
					// First generate template var -> type map.
					for (size_t j = 0; j < numTemplateArguments; j++) {
						const auto templateTypeValue = ConvertType(context, astTemplateArgs->at(j));
						templateVarMap.insert(typeInstance->templateVariables().at(j), templateTypeValue);
					}
					
					// Then check all the types are valid parameters.
					for (size_t j = 0; j < numTemplateArguments; j++) {
						const auto templateTypeValue = ConvertType(context, astTemplateArgs->at(j));
						
						if (templateTypeValue->isInterface()) {
							throw ErrorException(makeString("Cannot use abstract type '%s' "
								"as template parameter %llu for type '%s' at position %s.",
								templateTypeValue->getObjectType()->name().toString().c_str(),
								(unsigned long long) j,
								name.toString().c_str(),
								location.toString().c_str()));
						}
						
						const auto templateVariable = typeInstance->templateVariables().at(j);
						
						if (templateVariable->specType() != nullptr) {
							assert(templateVariable->specType()->isInterface());
							
							if (templateTypeValue->isAuto()) {
								// Presumably auto will always work...
								continue;
							}
							
							if (!templateTypeValue->isObjectOrTemplateVar()) {
								throw ErrorException(makeString("Non-object type '%s' cannot satisfy "
									"constraint for template parameter %llu of type '%s' at position %s.",
									templateTypeValue->toString().c_str(),
									(unsigned long long) j,
									name.toString().c_str(),
									location.toString().c_str()));
							}
							
							const auto specType = templateVariable->specType()->substitute(templateVarMap);
							
							if (!TypeSatisfiesInterface(templateTypeValue, specType)) {
								throw ErrorException(makeString("Type '%s' does not satisfy "
									"constraint for template parameter %llu of type '%s' at position %s.",
									templateTypeValue->getObjectType()->name().toString().c_str(),
									(unsigned long long) j,
									name.toString().c_str(),
									location.toString().c_str()));
							}
						}
					}
				} else {
					if (numTemplateArguments > 0) {
						throw ErrorException(makeString("%llu template "
							"arguments provided for non-type node '%s'; "
							"none should be provided at position %s.",
							(unsigned long long) numTemplateArguments,
							name.toString().c_str(),
							location.toString().c_str()));
					}
				}
			}
			
			return templateVarMap;
		}
		
		std::vector<SEM::Type*> GetTemplateValues(Context& context, const AST::Node<AST::Symbol>& astSymbol) {
			std::vector<SEM::Type*> templateArguments;
			for (size_t i = 0; i < astSymbol->size(); i++) {
				const auto& astSymbolElement = astSymbol->at(i);
				for (const auto& astTemplateArg: *(astSymbolElement->templateArguments())) {
					templateArguments.push_back(ConvertType(context, astTemplateArg));
				}
			}
			return templateArguments;
		}
		
		SEM::Type* ConvertIntegerType(Context& context, AST::Type::SignedModifier signedModifier, const std::string& nameString) {
			// Unsigned types have 'u' prefix and all integer types
			// have '_t' suffix (e.g. uint_t, short_t etc.).
			const auto fullNameString = (signedModifier == AST::Type::UNSIGNED) ? makeString("u%s_t", nameString.c_str()) : makeString("%s_t", nameString.c_str());
			return SEM::Type::Object(getBuiltInType(context.scopeStack(), fullNameString), SEM::Type::NO_TEMPLATE_ARGS);
		}
		
		SEM::Type* ConvertFloatType(Context& context, const std::string& nameString) {
			// All floating point types have '_t' suffix (e.g. float_t, double_t etc.).
			const auto fullNameString = makeString("%s_t", nameString.c_str());
			return SEM::Type::Object(getBuiltInType(context.scopeStack(), fullNameString), SEM::Type::NO_TEMPLATE_ARGS);
		}
		
		SEM::Type* ConvertObjectType(Context& context, const AST::Node<AST::Symbol>& symbol) {
			assert(!symbol->empty());
			
			const Name name = symbol->createName();
			
			const auto searchResult = performSearch(context, name);
			
			const auto templateVarMap = GenerateTemplateVarMap(context, symbol);
			
			if (searchResult.isTypeInstance()) {
				const auto typeInstance = searchResult.typeInstance();
				
				assert(templateVarMap.size() == typeInstance->templateVariables().size());
				
				std::vector<SEM::Type*> templateArguments;
				for(size_t i = 0; i < typeInstance->templateVariables().size(); i++){
					templateArguments.push_back(templateVarMap.get(typeInstance->templateVariables().at(i)));
				}
				
				return SEM::Type::Object(typeInstance, templateArguments);
			} else if(searchResult.isTemplateVar()) {
				assert(templateVarMap.empty());
				
				return SEM::Type::TemplateVarRef(searchResult.templateVar());
			} else {
				throw ErrorException(makeString("Unknown type with name '%s' at position %s.",
					name.toString().c_str(), symbol.location().toString().c_str()));
			}
		}
		
		SEM::Type* ConvertType(Context& context, const AST::Node<AST::Type>& type) {
			switch(type->typeEnum) {
				case AST::Type::AUTO: {
					return SEM::Type::Auto();
				}
				case AST::Type::BRACKET: {
					return ConvertType(context, type->getBracketTarget());
				}
				case AST::Type::CONST: {
					return ConvertType(context, type->getConstTarget())->createConstType();
				}
				case AST::Type::LVAL: {
					auto targetType = ConvertType(context, type->getLvalTarget());
					return ConvertType(context, type->getLvalType())->createLvalType(targetType);
				}
				case AST::Type::REF: {
					auto targetType = ConvertType(context, type->getRefTarget());
					return ConvertType(context, type->getRefType())->createRefType(targetType);
				}
				case AST::Type::VOID: {
					return SEM::Type::Void();
				}
				case AST::Type::INTEGER: {
					return ConvertIntegerType(context, type->integerType.signedModifier, type->integerType.name);
				}
				case AST::Type::FLOAT: {
					return ConvertFloatType(context, type->floatType.name);
				}
				case AST::Type::OBJECT: {
					return ConvertObjectType(context, type->objectType.symbol);
				}
				case AST::Type::REFERENCE: {
					const auto targetType = ConvertType(context, type->getReferenceTarget());
					return createReferenceType(context, targetType);
				}
				case AST::Type::FUNCTION: {
					const auto returnType = ConvertType(context, type->functionType.returnType);
					
					std::vector<SEM::Type*> parameterTypes;
					
					const auto& astParameterTypes = type->functionType.parameterTypes;
					for (const auto& astParamType: *astParameterTypes) {
						SEM::Type* paramType = ConvertType(context, astParamType);
						
						if(paramType->isVoid()) {
							throw ErrorException("Parameter type (inside function type) cannot be void.");
						}
						
						parameterTypes.push_back(paramType);
					}
					
					// Currently no syntax exists to express a method function type.
					const bool isDynamicMethod = false;
					
					// Currently no syntax exists to express a templated method function type.
					const bool isTemplatedMethod = false;
					
					// Currently no syntax exists to express a type with 'noexcept'.
					const bool isNoExcept = false;
					
					return SEM::Type::Function(type->functionType.isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, returnType, parameterTypes);
				}
				default:
					throw std::runtime_error("Unknown AST::Node<AST::Type> type enum.");
			}
		}
		
	}
	
}

