#include <cassert>

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			SEM::Value addDebugInfo(SEM::Value value, const Debug::SourceLocation& location) {
				Debug::ValueInfo valueInfo;
				valueInfo.location = location;
				value.setDebugInfo(valueInfo);
				return value;
			}
			
		}
		
		SEM::FunctionType simplifyFunctionType(Context& context, const SEM::FunctionType oldFunctionType) {
			const bool isVarArg = oldFunctionType.attributes().isVarArg();
			const bool isMethod = oldFunctionType.attributes().isMethod();
			const bool isTemplated = oldFunctionType.attributes().isTemplated();
			auto noexceptPredicate = reducePredicate(context, oldFunctionType.attributes().noExceptPredicate().copy());
			const auto returnType = oldFunctionType.returnType();
			const auto& argTypes = oldFunctionType.parameterTypes();
			
			return SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isMethod, isTemplated, std::move(noexceptPredicate)), returnType, argTypes.copy());
		}
		
		SEM::Value GetStaticMethod(Context& context, SEM::Value rawValue, const String& methodName, const Debug::SourceLocation& location) {
			auto value = derefOrBindValue(context, std::move(rawValue));
			assert(value.type()->isRef() && value.type()->isBuiltInReference());
			assert(value.type()->refTarget()->isStaticRef());
			const auto targetType = value.type()->refTarget()->staticRefTarget()->resolveAliases();
			
			if (!targetType->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Cannot get static method '%s' for non-object type '%s' at position %s.",
					methodName.c_str(), targetType->toString().c_str(), location.toString().c_str()));
			}
			
			const auto methodSet = getTypeMethodSet(context, targetType);
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = methodSet->find(canonicalMethodName);
			
			if (methodIterator == methodSet->end()) {
				throw ErrorException(makeString("Cannot find static method '%s' for type '%s' at position %s.",
					methodName.c_str(),
					targetType->toString().c_str(),
					location.toString().c_str()));
			}
			
			const auto& methodElement = methodIterator->second;
			
			if (!methodElement.isStatic()) {
				throw ErrorException(makeString("Cannot call non-static method '%s' for type '%s' at position %s.",
					methodName.c_str(),
					targetType->toString().c_str(),
					location.toString().c_str()));
			}
			
			auto& typeBuilder = context.typeBuilder();
			
			if (targetType->isObject()) {
				// Get the actual function so we can refer to it.
				const auto& function = targetType->getObjectType()->getFunction(canonicalMethodName);
				const auto functionTypeTemplateMap = targetType->generateTemplateVarMap();
				
				const auto functionType = simplifyFunctionType(context, function.type().substitute(functionTypeTemplateMap));
				const auto functionRefType = typeBuilder.getFunctionPointerType(functionType);
				
				auto functionRef = addDebugInfo(SEM::Value::FunctionRef(targetType, &function, {}, functionRefType), location);
				
				if (targetType->isInterface()) {
					const auto interfaceMethodType = typeBuilder.getStaticInterfaceMethodType(functionType);
					return addDebugInfo(SEM::Value::StaticInterfaceMethodObject(std::move(functionRef), std::move(value), interfaceMethodType), location);
				} else {
					return functionRef;
				}
			} else {
				const bool isTemplated = true;
				const auto functionType = typeBuilder.getFunctionPointerType(methodElement.createFunctionType(isTemplated));
				return addDebugInfo(SEM::Value::TemplateFunctionRef(targetType, methodName, functionType), location);
			}
		}
		
		SEM::Value GetMethod(Context& context, SEM::Value rawValue, const String& methodName, const Debug::SourceLocation& location) {
			return GetTemplatedMethod(context, std::move(rawValue), methodName, {}, location);
		}
		
		SEM::Value GetTemplatedMethod(Context& context, SEM::Value rawValue, const String& methodName, SEM::ValueArray templateArguments, const Debug::SourceLocation& location) {
			auto value = derefOrBindValue(context, tryDissolveValue(context, derefOrBindValue(context, std::move(rawValue)), location));
			const auto type = getDerefType(value.type())->resolveAliases();
			return GetTemplatedMethodWithoutResolution(context, std::move(value), type, methodName, std::move(templateArguments), location);
		}
		
		// Gets the method without dissolving or derefencing the object.
		SEM::Value GetSpecialMethod(Context& context, SEM::Value value, const String& methodName, const Debug::SourceLocation& location) {
			assert(value.type()->isRef() && value.type()->isBuiltInReference());
			const auto type = getSingleDerefType(value.type())->resolveAliases();
			return GetMethodWithoutResolution(context, std::move(value), type, methodName, location);
		}
		
		SEM::Value GetMethodWithoutResolution(Context& context, SEM::Value value, const SEM::Type* type, const String& methodName, const Debug::SourceLocation& location) {
			return GetTemplatedMethodWithoutResolution(context, std::move(value), type, methodName, {}, location);
		}
		
		SEM::Value GetTemplatedMethodWithoutResolution(Context& context, SEM::Value value, const SEM::Type* const type, const String& methodName, SEM::ValueArray templateArguments, const Debug::SourceLocation& location) {
			assert(value.type()->isRef() && value.type()->isBuiltInReference());
			if (!type->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Cannot get method '%s' for non-object type '%s' at position %s.",
					methodName.c_str(), type->toString().c_str(), location.toString().c_str()));
			}
			
			const auto methodSet = getTypeMethodSet(context, type);
			const auto& objectConstPredicate = methodSet->constPredicate();
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = methodSet->find(canonicalMethodName);
			
			if (methodIterator == methodSet->end()) {
				throw ErrorException(makeString("Cannot find method '%s' for type '%s' at position %s.",
					methodName.c_str(),
					type->toString().c_str(),
					location.toString().c_str()));
			}
			
			const auto& methodElement = methodIterator->second;
			if (methodElement.isStatic()) {
				throw ErrorException(makeString("Cannot access static method '%s' for value of type '%s' at position %s.",
					methodName.c_str(),
					type->toString().c_str(),
					location.toString().c_str()));
			}
			
			auto templateVariableAssignments = type->generateTemplateVarMap();
			
			const auto function = type->isObject() ? &(type->getObjectType()->getFunction(canonicalMethodName)) : nullptr;
			
			if (function != nullptr) {
				const auto& templateVariables = function->templateVariables();
				if (templateVariables.size() != templateArguments.size()) {
					// Try to apply some basic deduction...
					if (templateVariables.size() == 1 && templateArguments.size() == 0 &&
						function->constPredicate().isVariable() &&
						function->constPredicate().variableTemplateVar() == templateVariables[0]
					) {
						const auto boolType = context.typeBuilder().getBoolType();
						templateArguments.push_back(SEM::Value::PredicateExpr(objectConstPredicate.copy(), boolType));
					} else {
						throw ErrorException(makeString("Incorrect number of template "
							"arguments provided for method '%s'; %llu were required, "
							"but %llu were provided at position %s.",
							function->name().toString().c_str(),
							static_cast<unsigned long long>(templateVariables.size()),
							static_cast<unsigned long long>(templateArguments.size()),
							location.toString().c_str()));
					}
				}
				
				// Add function template variable => argument mapping.
				for (size_t i = 0; i < templateArguments.size(); i++) {
					const auto templateVariable = templateVariables.at(i);
					const auto& templateValue = templateArguments.at(i);
					
					if (templateValue.isTypeRef()) {
						const auto templateTypeValue = templateValue.typeRefType()->resolveAliases();
						
						if (!templateTypeValue->isObjectOrTemplateVar() || templateTypeValue->isInterface()) {
							throw ErrorException(makeString("Invalid type '%s' passed "
								"as template parameter '%s' for method '%s' at position %s.",
								templateTypeValue->toString().c_str(),
								templateVariable->name().toString().c_str(),
								function->name().toString().c_str(),
								location.toString().c_str()));
						}
						
						templateVariableAssignments.insert(std::make_pair(templateVariable, SEM::Value::TypeRef(templateTypeValue, templateValue.type())));
					} else {
						templateVariableAssignments.insert(std::make_pair(templateVariable, templateValue.copy()));
					}
				}
			} else {
				assert(templateArguments.empty());
			}
			
			const auto methodConstPredicate = methodElement.constPredicate().substitute(templateVariableAssignments);
			
			if (!objectConstPredicate.implies(methodConstPredicate)) {
				throw ErrorException(makeString("Cannot refer to mutator method '%s' from const object of type '%s' at position %s.",
					methodName.c_str(),
					type->toString().c_str(),
					location.toString().c_str()));
			}
			
			// Now check the template arguments satisfy the requires predicate.
			const auto& requirePredicate = methodElement.requirePredicate();
			
			// Conservatively assume require predicate is not satisified if result is undetermined.
			const bool satisfiesRequireDefault = false;
			
			if (!evaluatePredicateWithDefault(context, requirePredicate, templateVariableAssignments, satisfiesRequireDefault)) {
				throw ErrorException(makeString("Template arguments do not satisfy "
					"require predicate '%s' of method '%s' at position %s.",
					requirePredicate.substitute(templateVariableAssignments).toString().c_str(),
					methodName.c_str(),
					location.toString().c_str()));
			}
			
			auto& typeBuilder = context.typeBuilder();
			
			if (function != nullptr) {
				const auto functionType = simplifyFunctionType(context, function->type().substitute(templateVariableAssignments));
				const auto functionRefType = typeBuilder.getFunctionPointerType(functionType);
				
				auto functionRef = addDebugInfo(SEM::Value::FunctionRef(type, function, std::move(templateArguments), functionRefType), location);
				
				if (type->isInterface()) {
					const auto interfaceMethodType = typeBuilder.getInterfaceMethodType(functionType);
					return addDebugInfo(SEM::Value::InterfaceMethodObject(std::move(functionRef), std::move(value), interfaceMethodType), location);
				} else {
					const auto methodType = typeBuilder.getMethodType(functionType);
					return addDebugInfo(SEM::Value::MethodObject(std::move(functionRef), std::move(value), methodType), location);
				}
			} else {
				const bool isTemplated = true;
				const auto functionType = methodElement.createFunctionType(isTemplated);
				const auto functionRefType = typeBuilder.getFunctionPointerType(functionType);
				auto functionRef = addDebugInfo(SEM::Value::TemplateFunctionRef(type, methodName, functionRefType), location);
				
				const auto methodType = typeBuilder.getMethodType(functionType);
				return addDebugInfo(SEM::Value::MethodObject(std::move(functionRef), std::move(value), methodType), location);
			}
		}
		
	}
	
}


