#include <stdio.h>

#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/Template.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		const SEM::Type* ConvertIntegerType(Context& context, AST::Type::SignedModifier signedModifier, const String& nameString) {
			// Unsigned types have 'u' prefix and all integer types
			// have '_t' suffix (e.g. uint_t, short_t etc.).
			const auto fullNameString = (signedModifier == AST::Type::UNSIGNED) ? (context.getCString("u") + nameString + "_t") : (nameString + "_t");
			return getBuiltInType(context, fullNameString, {});
		}
		
		const SEM::Type* ConvertFloatType(Context& context, const String& nameString) {
			// All floating point types have '_t' suffix (e.g. float_t, double_t etc.).
			const auto fullNameString = nameString + "_t";
			return getBuiltInType(context, fullNameString, {});
		}
		
		const SEM::Type* ConvertObjectType(Context& context, const AST::Node<AST::Symbol>& symbol) {
			assert(!symbol->empty());
			
			const Name name = symbol->createName();
			
			const auto searchResult = performSearch(context, name);
			
			const auto templateVarMap = GenerateSymbolTemplateVarMap(context, symbol);
			
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
			return getBuiltInType(context, context.getCString("__ptr"), { varType });
		}
		
		SEM::ValueArray getFunctionTemplateArgs(Context& context, const SEM::FunctionType functionType) {
			const auto& parameterTypes = functionType.parameterTypes();
			
			SEM::ValueArray templateArgs;
			templateArgs.reserve(1 + parameterTypes.size());
			
			const auto boolType = getBuiltInType(context, context.getCString("bool"), {});
			templateArgs.push_back(SEM::Value::PredicateExpr(functionType.attributes().noExceptPredicate().copy(), boolType));
			
			const auto typenameType = getBuiltInType(context, context.getCString("typename_t"), {});
			
			const auto returnType = functionType.returnType();
			templateArgs.push_back(SEM::Value::TypeRef(returnType, typenameType->createStaticRefType(returnType)));
			
			for (const auto& paramType: parameterTypes) {
				templateArgs.push_back(SEM::Value::TypeRef(paramType, typenameType->createStaticRefType(paramType)));
			}
			
			return templateArgs;
		}
		
		const SEM::Type* createPrimitiveCallableType(Context& context, const SEM::FunctionType functionType, const std::string& prefix, const std::string& suffix) {
			const auto& parameterTypes = functionType.parameterTypes();
			const auto functionTypeName = makeString("%s%llu_%s", prefix.c_str(), static_cast<unsigned long long>(parameterTypes.size()), suffix.c_str());
			return getBuiltInTypeWithValueArgs(context, context.getString(functionTypeName), getFunctionTemplateArgs(context, functionType));
		}
		
		const SEM::Type* createTrivialFunctionPointerType(Context& context, const SEM::FunctionType functionType) {
			return createPrimitiveCallableType(context, functionType, "function", "ptr_t");
		}
		
		const SEM::Type* createTemplatedFunctionPointerType(Context& context, const SEM::FunctionType functionType) {
			return createPrimitiveCallableType(context, functionType, "templatedfunction", "ptr_t");
		}
		
		const SEM::Type* createMethodFunctionPointerType(Context& context, const SEM::FunctionType functionType) {
			return createPrimitiveCallableType(context, functionType, "methodfunction", "ptr_t");
		}
		
		const SEM::Type* createTemplatedMethodFunctionPointerType(Context& context, const SEM::FunctionType functionType) {
			return createPrimitiveCallableType(context, functionType, "templatedmethodfunction", "ptr_t");
		}
		
		const SEM::Type* createVarArgFunctionPointerType(Context& context, const SEM::FunctionType functionType) {
			return createPrimitiveCallableType(context, functionType, "varargfunction", "ptr_t");
		}
		
		const SEM::Type* createFunctionPointerType(Context& context, const SEM::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			
			if (attributes.isVarArg()) {
				return createVarArgFunctionPointerType(context, functionType);
			} else if (attributes.isMethod()) {
				if (attributes.isTemplated()) {
					return createTemplatedMethodFunctionPointerType(context, functionType);
				} else {
					return createMethodFunctionPointerType(context, functionType);
				}
			} else {
				if (attributes.isTemplated()) {
					return createTemplatedFunctionPointerType(context, functionType);
				} else {
					return createTrivialFunctionPointerType(context, functionType);
				}
			}
		}
		
		const SEM::Type* createTrivialMethodType(Context& context, const SEM::FunctionType functionType) {
			return createPrimitiveCallableType(context, functionType, "method", "t");
		}
		
		const SEM::Type* createTemplatedMethodType(Context& context, const SEM::FunctionType functionType) {
			return createPrimitiveCallableType(context, functionType, "templatedmethod", "t");
		}
		
		const SEM::Type* createMethodType(Context& context, const SEM::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			assert(!attributes.isVarArg());
			assert(attributes.isMethod());
			
			if (attributes.isTemplated()) {
				return createTemplatedMethodType(context, functionType);
			} else {
				return createTrivialMethodType(context, functionType);
			}
		}
		
		const SEM::Type* createInterfaceMethodType(Context& context, const SEM::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			(void) attributes;
			assert(!attributes.isVarArg());
			assert(attributes.isMethod());
			
			return createPrimitiveCallableType(context, functionType, "interfacemethod", "t");
		}
		
		const SEM::Type* createStaticInterfaceMethodType(Context& context, const SEM::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			(void) attributes;
			assert(!attributes.isVarArg());
			assert(!attributes.isMethod());
			
			return createPrimitiveCallableType(context, functionType, "staticinterfacemethod", "t");
		}
		
		const SEM::Type* ConvertType(Context& context, const AST::Node<AST::Type>& type) {
			switch(type->typeEnum) {
				case AST::Type::AUTO: {
					return SEM::Type::Auto(context.semContext());
				}
				case AST::Type::CONST: {
					return ConvertType(context, type->getConstTarget())->createTransitiveConstType(SEM::Predicate::True());
				}
				case AST::Type::CONSTPREDICATE: {
					auto constPredicate = ConvertPredicate(context, type->getConstPredicate());
					const auto constTarget = ConvertType(context, type->getConstPredicateTarget());
					return constTarget->createTransitiveConstType(std::move(constPredicate));
				}
				case AST::Type::NOTAG: {
					return ConvertType(context, type->getNoTagTarget())->createNoTagType();
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
					return getBuiltInType(context, context.getCString("void_t"), {});
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
					auto noexceptPredicate = SEM::Predicate::False();
					
					const bool isVarArg = type->functionType.isVarArg;
					
					SEM::FunctionAttributes attributes(isVarArg, isDynamicMethod, isTemplated, std::move(noexceptPredicate));
					const SEM::FunctionType builtInFunctionType(std::move(attributes),  returnType, std::move(parameterTypes));
					
					return createFunctionPointerType(context, builtInFunctionType);
				}
			}
			
			std::terminate();
		}
		
	}
	
}

