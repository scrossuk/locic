#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			std::vector<SEM::Value*> CastFunctionArguments(Context& context, const std::vector<SEM::Value*>& arguments, const std::vector<SEM::Type*>& types, const Debug::SourceLocation& location) {
				std::vector<SEM::Value*> castValues;
				castValues.reserve(arguments.size());
				
				for (size_t i = 0; i < arguments.size(); i++) {
					const auto argumentValue = arguments.at(i);
					
					// Cast arguments to the function type's corresponding
					// argument type; var-arg arguments should be cast to
					// one of the allowed types (since there's no specific
					// destination type).
					const auto castArgumentValue = (i < types.size()) ?
						ImplicitCast(context, argumentValue, types.at(i), location) :
						VarArgCast(context, argumentValue, location);
					
					castValues.push_back(castArgumentValue);
				}
				
				return castValues;
			}
			
			bool isCallableType(SEM::Type* type) {
				switch (type->kind()) {
					case SEM::Type::FUNCTION:
					case SEM::Type::METHOD:
					case SEM::Type::INTERFACEMETHOD:
					case SEM::Type::STATICINTERFACEMETHOD:
						return true;
					default:
						return false;
				}
			}
			
		}
		
		SEM::Value* GetStaticMethod(Context&, SEM::Value* rawValue, const std::string& methodName, const Debug::SourceLocation& location) {
			const auto value = derefAll(rawValue);
			const auto targetType = value->type()->staticRefTarget();
			
			if (!targetType->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Cannot get static method '%s' for non-object type '%s' at position %s.",
					methodName.c_str(), targetType->toString().c_str(), location.toString().c_str()));
			}
			
			const auto typeInstance = targetType->isObject() ? targetType->getObjectType() : targetType->getTemplateVar()->specTypeInstance();
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = typeInstance->functions().find(canonicalMethodName);
			
			if (methodIterator == typeInstance->functions().end()) {
				throw ErrorException(makeString("Cannot find static method '%s' for type '%s' at position %s.",
					methodName.c_str(), typeInstance->refToString().c_str(), location.toString().c_str()));
			}
			
			const auto function = methodIterator->second;
			assert(function->isMethod());
			
			if (!function->isStaticMethod()) {
				throw ErrorException(makeString("Cannot call non-static method '%s' for type '%s' at position %s.",
					methodName.c_str(), typeInstance->refToString().c_str(), location.toString().c_str()));
			}
			
			const auto functionRef = SEM::Value::FunctionRef(targetType, function, {}, targetType->generateTemplateVarMap());
			
			if (targetType->isInterface()) {
				return SEM::Value::StaticInterfaceMethodObject(functionRef, value);
			} else {
				return functionRef;
			}
		}
		
		SEM::Value* GetMethod(Context& context, SEM::Value* rawValue, const std::string& methodName, const Debug::SourceLocation& location) {
			return GetTemplatedMethod(context, rawValue, methodName, {}, location);
		}
		
		SEM::Value* GetTemplatedMethod(Context& context, SEM::Value* rawValue, const std::string& methodName, const std::vector<SEM::Type*>& templateArguments, const Debug::SourceLocation& location) {
			const auto value = tryDissolveValue(context, derefValue(rawValue), location);
			const auto type = getDerefType(value->type())->resolveAliases();
			
			return GetTemplatedMethodWithoutResolution(context, value, type, methodName, templateArguments, location);
		}
		
		// Gets the method without dissolving or derefencing the object.
		SEM::Value* GetSpecialMethod(Context& context, SEM::Value* value, const std::string& methodName, const Debug::SourceLocation& location) {
			return GetMethodWithoutResolution(context, value, getSingleDerefType(value->type())->resolveAliases(), methodName, location);
		}
		
		SEM::Value* GetMethodWithoutResolution(Context& context, SEM::Value* value, SEM::Type* type, const std::string& methodName, const Debug::SourceLocation& location) {
			return GetTemplatedMethodWithoutResolution(context, value, type, methodName, {}, location);
		}
		
		SEM::Value* GetTemplatedMethodWithoutResolution(Context&, SEM::Value* value, SEM::Type* type, const std::string& methodName, const std::vector<SEM::Type*>& templateArguments, const Debug::SourceLocation& location) {
			if (!type->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Cannot get method '%s' for non-object type '%s' at position %s.",
					methodName.c_str(), type->toString().c_str(), location.toString().c_str()));
			}
			
			const auto typeInstance = type->isObject() ? type->getObjectType() : type->getTemplateVar()->specTypeInstance();
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = typeInstance->functions().find(canonicalMethodName);
			
			if (methodIterator == typeInstance->functions().end()) {
				throw ErrorException(makeString("Cannot find method '%s' for type '%s' at position %s.",
					methodName.c_str(), type->toString().c_str(), location.toString().c_str()));
			}
			
			const auto function = methodIterator->second;
			assert(function->isMethod());
			
			if (function->isStaticMethod()) {
				throw ErrorException(makeString("Cannot access static method '%s' for value of type '%s' at position %s.",
					methodName.c_str(),
					typeInstance->refToString().c_str(), location.toString().c_str()));
			}
			
			if (type->isConst() && !function->isConstMethod()) {
				throw ErrorException(makeString("Cannot refer to mutator method '%s' from const object of type '%s' at position %s.",
					function->name().toString().c_str(),
					type->toString().c_str(), location.toString().c_str()));
			}
			
			const auto templateVariables = function->templateVariables();
			if (templateVariables.size() != templateArguments.size()) {
				throw ErrorException(makeString("Incorrect number of template "
					"arguments provided for method '%s'; %llu were required, "
					"but %llu were provided at position %s.",
					function->name().toString().c_str(),
					(unsigned long long) templateVariables.size(),
					(unsigned long long) templateArguments.size(),
					location.toString().c_str()));
			}
			
			// Create map from variables to values for both the method
			// and its parent type.
			auto combinedTemplateVarMap = type->generateTemplateVarMap();
			for (size_t i = 0; i < templateArguments.size(); i++) {
				combinedTemplateVarMap.insert(std::make_pair(templateVariables.at(i), templateArguments.at(i)));
			}
			
			// Now check all the arguments are valid.
			for (size_t i = 0; i < templateArguments.size(); i++) {
				const auto templateVariable = templateVariables.at(i);
				const auto templateTypeValue = templateArguments.at(i)->resolveAliases();
				
				if (!templateTypeValue->isObjectOrTemplateVar() || templateTypeValue->isInterface()) {
					throw ErrorException(makeString("Invalid type '%s' passed "
						"as template parameter %llu for method '%s' at position %s.",
						templateTypeValue->toString().c_str(),
						(unsigned long long) i,
						function->name().toString().c_str(),
						location.toString().c_str()));
				}
				
				if (templateVariable->specType() != nullptr) {
					assert(templateVariable->specType()->isInterface());
					
					const auto specType = templateVariable->specType()->substitute(combinedTemplateVarMap);
					
					if (!TypeSatisfiesInterface(templateTypeValue, specType)) {
						throw ErrorException(makeString("Type '%s' does not satisfy "
							"constraint for template parameter %llu of method '%s' at position %s.",
							templateTypeValue->getObjectType()->name().toString().c_str(),
							(unsigned long long) i,
							function->name().toString().c_str(),
							location.toString().c_str()));
					}
				}
			}
			
			const auto functionRef = SEM::Value::FunctionRef(type, function, templateArguments, combinedTemplateVarMap);
			
			if (typeInstance->isInterface()) {
				return SEM::Value::InterfaceMethodObject(functionRef, derefValue(value));
			} else {
				return SEM::Value::MethodObject(functionRef, derefValue(value));
			}
		}
		
		SEM::Value* CallValue(Context& context, SEM::Value* rawValue, const std::vector<SEM::Value*>& args, const Debug::SourceLocation& location) {
			const auto value = tryDissolveValue(context, derefValue(rawValue), location);
			
			if (getDerefType(value->type())->isStaticRef()) {
				return CallValue(context, GetStaticMethod(context, value, "create", location), args, location);
			}
			
			if (!isCallableType(value->type())) {
				throw ErrorException(makeString("Can't call value '%s' that isn't a function or a method at position %s.",
					value->toString().c_str(), location.toString().c_str()));
			}
			
			const auto functionType = value->type()->getCallableFunctionType();
			const auto& typeList = functionType->getFunctionParameterTypes();
			
			if (functionType->isFunctionVarArg()) {
				if (args.size() < typeList.size()) {
					throw ErrorException(makeString("Var Arg Function [%s] called with %llu "
						"parameters; expected at least %llu at position %s.",
						value->toString().c_str(),
						(unsigned long long) args.size(),
						(unsigned long long) typeList.size(),
						location.toString().c_str()));
				}
			} else {
				if (args.size() != typeList.size()) {
					throw ErrorException(makeString("Function [%s] called with %llu "
						"parameters; expected %llu at position %s.",
						value->toString().c_str(),
						(unsigned long long) args.size(),
						(unsigned long long) typeList.size(),
						location.toString().c_str()));
				}
			}
			
			return SEM::Value::FunctionCall(value, CastFunctionArguments(context, args, typeList, location));
		}
		
		bool supportsNullConstruction(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
					return false;
					
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find("null");
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->isMethod()) return false;
					if (!function->isStaticMethod()) return false;
					if (function->isConstMethod()) return false;
					if (!function->parameters().empty()) return false;
					
					return true;
				}
				
				case SEM::Type::TEMPLATEVAR:
					return supportsNullConstruction(type->getTemplateVar()->specTypeInstance()->selfType());
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
		bool supportsImplicitCast(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
				case SEM::Type::TEMPLATEVAR:
					return false;
					
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find("implicitcast");
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					if (!function->isConstMethod()) return false;
					if (!function->parameters().empty()) return false;
					if (function->templateVariables().size() != 1) return false;
					
					const auto returnType = function->type()->getFunctionReturnType()->substitute(type->generateTemplateVarMap());
					
					if (!returnType->isTemplateVar()) return false;
					if (returnType->getTemplateVar() != function->templateVariables().front()) return false;
					
					return true;
				}
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
		bool supportsCopy(SEM::Type* const type, const std::string& functionName) {
			assert(!type->isAlias());
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
				case SEM::Type::STATICINTERFACEMETHOD:
					// Built-in types can be copied.
					return true;
					
				case SEM::Type::OBJECT: {
					// Named types must have a method for copying.
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find(functionName);
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					if (!function->isConstMethod()) return false;
					if (!function->parameters().empty()) return false;
					
					const auto returnType = function->type()->getFunctionReturnType()->substitute(type->generateTemplateVarMap());
					if (returnType->isLvalOrRef()) return false;
					
					return true;
				}
				
				case SEM::Type::TEMPLATEVAR:
					return supportsCopy(type->getTemplateVar()->specTypeInstance()->selfType(), functionName);
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
		bool supportsNoExceptCopy(SEM::Type* const type, const std::string& functionName) {
			assert(!type->isAlias());
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
				case SEM::Type::STATICINTERFACEMETHOD:
					// Built-in types can be copied noexcept.
					return true;
					
				case SEM::Type::OBJECT: {
					// Named types must have a method for copying noexcept.
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find(functionName);
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->type()->isFunctionNoExcept()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					if (!function->isConstMethod()) return false;
					if (!function->parameters().empty()) return false;
					
					const auto returnType = function->type()->getFunctionReturnType()->substitute(type->generateTemplateVarMap());
					if (returnType->isLvalOrRef()) return false;
					
					return true;
				}
				
				case SEM::Type::TEMPLATEVAR:
					return supportsNoExceptCopy(type->getTemplateVar()->specTypeInstance()->selfType(), functionName);
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
		bool supportsImplicitCopy(SEM::Type* const type) {
			return supportsCopy(type->resolveAliases(), "implicitcopy");
		}
		
		bool supportsNoExceptImplicitCopy(SEM::Type* const type) {
			return supportsNoExceptCopy(type->resolveAliases(), "implicitcopy");
		}
		
		bool supportsExplicitCopy(SEM::Type* const type) {
			return supportsCopy(type->resolveAliases(), "copy");
		}
		
		bool supportsNoExceptExplicitCopy(SEM::Type* const type) {
			return supportsNoExceptCopy(type->resolveAliases(), "copy");
		}
		
		bool supportsCompare(SEM::Type* const rawType) {
			SEM::Type* const type = rawType->resolveAliases();
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
				case SEM::Type::STATICINTERFACEMETHOD:
					return false;
					
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find("compare");
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					if (!function->isConstMethod()) return false;
					if (function->parameters().size() != 1) return false;
					
					const auto firstArgType = function->parameters().at(0)->constructType();
					if (!firstArgType->isRef()) return false;
					if (!firstArgType->isBuiltInReference()) return false;
					if (!firstArgType->refTarget()->isConst()) return false;
					if (firstArgType->refTarget() != type->createConstType()) return false;
					
					return true;
				}
				
				case SEM::Type::TEMPLATEVAR:
					return supportsCompare(type->getTemplateVar()->specTypeInstance()->selfType());
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
	}
	
}


