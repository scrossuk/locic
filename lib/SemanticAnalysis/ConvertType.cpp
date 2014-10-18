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
						
						if (templateVariable->specType() != nullptr) {
							assert(templateVariable->specType()->isInterface());
							
							if (!templateTypeValue->isObjectOrTemplateVar()) {
								throw ErrorException(makeString("Non-object type '%s' cannot satisfy "
									"constraint for template parameter %llu of function or type '%s' at position %s.",
									templateTypeValue->toString().c_str(),
									(unsigned long long) j,
									name.toString().c_str(),
									location.toString().c_str()));
							}
							
							const auto specType = templateVariable->specType()->substitute(templateVarMap);
							
							if (!TypeSatisfiesInterface(templateTypeValue, specType)) {
								throw ErrorException(makeString("Type '%s' does not satisfy "
									"constraint for template parameter %llu of function or type '%s' at position %s.",
									templateTypeValue->getObjectType()->name().toString().c_str(),
									(unsigned long long) j,
									name.toString().c_str(),
									location.toString().c_str()));
							}
						} else {
							// Record this instantiation to be checked later.
							context.templateInstantiations().push_back(std::make_tuple(templateVariable, templateTypeValue, templateVarMap, name, location));
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
		
		const SEM::Type* ConvertIntegerType(Context& context, AST::Type::SignedModifier signedModifier, const std::string& nameString) {
			// Unsigned types have 'u' prefix and all integer types
			// have '_t' suffix (e.g. uint_t, short_t etc.).
			const auto fullNameString = (signedModifier == AST::Type::UNSIGNED) ? makeString("u%s_t", nameString.c_str()) : makeString("%s_t", nameString.c_str());
			return getBuiltInType(context.scopeStack(), fullNameString);
		}
		
		const SEM::Type* ConvertFloatType(Context& context, const std::string& nameString) {
			// All floating point types have '_t' suffix (e.g. float_t, double_t etc.).
			const auto fullNameString = makeString("%s_t", nameString.c_str());
			return getBuiltInType(context.scopeStack(), fullNameString);
		}
		
		const SEM::Type* ConvertObjectType(Context& context, const AST::Node<AST::Symbol>& symbol) {
			assert(!symbol->empty());
			
			const Name name = symbol->createName();
			
			const auto searchResult = performSearch(context, name);
			
			const auto templateVarMap = GenerateTemplateVarMap(context, symbol);
			
			if (searchResult.isTypeInstance()) {
				const auto typeInstance = searchResult.typeInstance();
				
				assert(templateVarMap.size() == typeInstance->templateVariables().size());
				
				return SEM::Type::Object(typeInstance, GetTemplateValues(templateVarMap, typeInstance->templateVariables()));
			} else if (searchResult.isTemplateVar()) {
				assert(templateVarMap.empty());
				
				return SEM::Type::TemplateVarRef(searchResult.templateVar());
			} else if (searchResult.isTypeAlias()) {
				const auto typeAlias = searchResult.typeAlias();
				
				assert(templateVarMap.size() == typeAlias->templateVariables().size());
				
				const auto templateValues = GetTemplateValues(templateVarMap, typeAlias->templateVariables());
				assert(templateValues.size() == typeAlias->templateVariables().size());
				
				return SEM::Type::Alias(typeAlias, templateValues);
			} else {
				throw ErrorException(makeString("Unknown type with name '%s' at position %s.",
					name.toString().c_str(), symbol.location().toString().c_str()));
			}
		}
		
		const SEM::Type* createPointerType(Context& context, const SEM::Type* varType) {
			const auto pointerTypeInst = getBuiltInType(context.scopeStack(), "__ptr")->getObjectType();
			return SEM::Type::Object(pointerTypeInst, { varType});
		}
		
		const SEM::Type* ConvertType(Context& context, const AST::Node<AST::Type>& type) {
			switch(type->typeEnum) {
				case AST::Type::AUTO: {
					return SEM::Type::Auto(context.semContext());
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
				case AST::Type::STATICREF: {
					auto targetType = ConvertType(context, type->getStaticRefTarget());
					return ConvertType(context, type->getStaticRefType())->createStaticRefType(targetType);
				}
				case AST::Type::VOID: {
					return getBuiltInType(context.scopeStack(), "void_t");
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
				case AST::Type::POINTER: {
					const auto targetType = ConvertType(context, type->getPointerTarget());
					return createPointerType(context, targetType);
				}
				case AST::Type::FUNCTION: {
					const auto returnType = ConvertType(context, type->functionType.returnType);
					
					const auto& astParameterTypes = type->functionType.parameterTypes;
					
					std::vector<const SEM::Type*> parameterTypes;
					parameterTypes.reserve(astParameterTypes->size());
					
					for (const auto& astParamType: *astParameterTypes) {
						const auto paramType = ConvertType(context, astParamType);
						
						if(paramType->isBuiltInVoid()) {
							throw ErrorException("Parameter type (inside function type) cannot be void.");
						}
						
						parameterTypes.push_back(paramType);
					}
					
					// Currently no syntax exists to express a method function type.
					const bool isDynamicMethod = false;
					
					// Currently no syntax exists to express a templated function type.
					const bool isTemplated = false;
					
					// Currently no syntax exists to express a type with 'noexcept'.
					const bool isNoExcept = false;
					
					return SEM::Type::Function(type->functionType.isVarArg, isDynamicMethod, isTemplated, isNoExcept, returnType, parameterTypes);
				}
			}
			
			std::terminate();
		}
		
	}
	
}

