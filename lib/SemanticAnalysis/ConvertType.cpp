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
	
		const SEM::Type* ConvertIntegerType(Context& context, AST::Type::SignedModifier signedModifier, const String& nameString) {
			// Unsigned types have 'u' prefix and all integer types
			// have '_t' suffix (e.g. uint_t, short_t etc.).
			const auto fullNameString = (signedModifier == AST::Type::UNSIGNED) ? (context.getCString("u") + nameString + "_t") : (nameString + "_t");
			return getBuiltInType(context.scopeStack(), fullNameString, {});
		}
		
		const SEM::Type* ConvertFloatType(Context& context, const String& nameString) {
			// All floating point types have '_t' suffix (e.g. float_t, double_t etc.).
			const auto fullNameString = nameString + "_t";
			return getBuiltInType(context.scopeStack(), fullNameString, {});
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
				
				auto templateValues = GetTemplateValues(templateVarMap, typeAlias->templateVariables());
				assert(templateValues.size() == typeAlias->templateVariables().size());
				
				return SEM::Type::Alias(typeAlias, std::move(templateValues));
			} else {
				throw ErrorException(makeString("Unknown type with name '%s' at position %s.",
					name.toString().c_str(), symbol.location().toString().c_str()));
			}
		}
		
		const SEM::Type* createPointerType(Context& context, const SEM::Type* const varType) {
			return getBuiltInType(context.scopeStack(), context.getCString("__ptr"), { varType });
		}
		
		const SEM::Type* ConvertType(Context& context, const AST::Node<AST::Type>& type) {
			switch(type->typeEnum) {
				case AST::Type::AUTO: {
					return SEM::Type::Auto(context.semContext());
				}
				case AST::Type::CONST: {
					return ConvertType(context, type->getConstTarget())->createConstType();
				}
				case AST::Type::CONSTPREDICATE: {
					throw std::logic_error("Const predicate type not yet implemented.");
				}
				case AST::Type::MUTABLE: {
					// Mutable is the default so doesn't affect the type.
					return ConvertType(context, type->getMutableTarget());
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
					return getBuiltInType(context.scopeStack(), context.getCString("void_t"), {});
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
					
					SEM::TypeArray parameterTypes;
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
					
					return SEM::Type::Function(type->functionType.isVarArg, isDynamicMethod, isTemplated, isNoExcept, returnType, std::move(parameterTypes));
				}
			}
			
			std::terminate();
		}
		
	}
	
}

