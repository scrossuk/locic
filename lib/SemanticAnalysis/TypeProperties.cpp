#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			HeapArray<SEM::Value> CastFunctionArguments(Context& context, HeapArray<SEM::Value> arguments, const SEM::TypeArray& types, const Debug::SourceLocation& location) {
				HeapArray<SEM::Value> castValues;
				castValues.reserve(arguments.size());
				
				for (size_t i = 0; i < arguments.size(); i++) {
					auto& argumentValue = arguments.at(i);
					
					// Cast arguments to the function type's corresponding
					// argument type; var-arg arguments should be cast to
					// one of the allowed types (since there's no specific
					// destination type).
					auto castArgumentValue = (i < types.size()) ?
						ImplicitCast(context, std::move(argumentValue), types.at(i), location) :
						VarArgCast(context, std::move(argumentValue), location);
					
					castValues.push_back(std::move(castArgumentValue));
				}
				
				return castValues;
			}
			
			bool isCallableType(const SEM::Type* const type) {
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
			
			SEM::Value addDebugInfo(SEM::Value value, const Debug::SourceLocation& location) {
				Debug::ValueInfo valueInfo;
				valueInfo.location = location;
				value.setDebugInfo(valueInfo);
				return value;
			}
			
		}
		
		SEM::Value GetStaticMethod(Context& context, SEM::Value rawValue, const String& methodName, const Debug::SourceLocation& location) {
			auto value = derefOrBindValue(context, std::move(rawValue));
			assert(value.type()->isRef() && value.type()->isBuiltInReference());
			assert(value.type()->refTarget()->isStaticRef());
			const auto targetType = value.type()->refTarget()->staticRefTarget();
			
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
			
			if (targetType->isObject()) {
				// Get the actual function so we can refer to it.
				const auto function = targetType->getObjectType()->functions().at(canonicalMethodName);
				const auto functionTypeTemplateMap = targetType->generateTemplateVarMap();
				auto functionRef = addDebugInfo(SEM::Value::FunctionRef(targetType, function, {}, function->type()->substitute(functionTypeTemplateMap)), location);
				
				if (targetType->isInterface()) {
					return addDebugInfo(SEM::Value::StaticInterfaceMethodObject(std::move(functionRef), std::move(value)), location);
				} else {
					return functionRef;
				}
			} else {
				const bool isTemplated = true;
				const auto functionType = methodElement.createFunctionType(isTemplated);
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
			
			// TODO: only get method set for template variables, since object
			// methods can have templated methods.
			const auto methodSet = getTypeMethodSet(context, type);
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = methodSet->find(canonicalMethodName);
			
			if (methodIterator == methodSet->end()) {
				// The method may have been filtered out, so let's find out why.
				const auto filterReason = methodSet->getFilterReason(canonicalMethodName);
				
				if (filterReason == MethodSet::IsMutator) {
					// Only check for template variables.
					if (!type->isObject()) {
						throw ErrorException(makeString("Cannot refer to mutator method '%s' from const object of type '%s' at position %s.",
							methodName.c_str(),
							type->toString().c_str(),
							location.toString().c_str()));
					}
				} else {
					throw ErrorException(makeString("Cannot find method '%s' for type '%s' at position %s.",
						methodName.c_str(),
						type->toString().c_str(),
						location.toString().c_str()));
				}
			}
			
			const auto& methodElement = methodIterator->second;
			if (methodElement.isStatic()) {
				throw ErrorException(makeString("Cannot access static method '%s' for value of type '%s' at position %s.",
					methodName.c_str(),
					type->toString().c_str(),
					location.toString().c_str()));
			}
			
			if (type->isObject()) {
				// Get the actual function so we can check its template arguments and refer to it.
				const auto function = type->getObjectType()->functions().at(canonicalMethodName);
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
				auto templateVariableAssignments = type->generateTemplateVarMap();
				for (size_t i = 0; i < templateArguments.size(); i++) {
					const auto templateVariable = templateVariables.at(i);
					auto& templateValue = templateArguments.at(i);
					
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
						
						templateVariableAssignments.insert(std::make_pair(templateVariable, SEM::Value::TypeRef(templateTypeValue, templateArguments.at(i).type())));
					} else {
						templateVariableAssignments.insert(std::make_pair(templateVariable, std::move(templateValue)));
					}
				}
				
				// Now check the template arguments satisfy the requires predicate.
				const auto& requiresPredicate = function->requiresPredicate();
				
				// Conservatively assume require predicate is not satisified if result is undetermined.
				const bool satisfiesRequiresDefault = false;
				
				if (!evaluatePredicateWithDefault(context, requiresPredicate, templateVariableAssignments, satisfiesRequiresDefault)) {
					throw ErrorException(makeString("Template arguments do not satisfy "
						"requires predicate '%s' of method '%s' at position %s.",
						requiresPredicate.toString().c_str(),
						function->name().toString().c_str(),
						location.toString().c_str()));
				}
				
				// Conservatively assume object type is const if result is undetermined.
				const bool isConstObjectDefault = true;
				
				const bool isConstObject = evaluatePredicateWithDefault(context, type->constPredicate(), templateVariableAssignments, isConstObjectDefault);
				
				// Conservatively assume method is not const if result is undetermined.
				const bool isConstMethodDefault = false;
				
				const bool isConstMethod = evaluatePredicateWithDefault(context, function->constPredicate(), templateVariableAssignments, isConstMethodDefault);
				
				if (isConstObject && !isConstMethod) {
					throw ErrorException(makeString("Cannot refer to mutator method '%s' from const object of type '%s' at position %s.",
						methodName.c_str(),
						type->toString().c_str(),
						location.toString().c_str()));
				}
				
				auto functionRef = addDebugInfo(SEM::Value::FunctionRef(type, function, std::move(templateArguments), function->type()->substitute(templateVariableAssignments)), location);
				
				if (type->isInterface()) {
					return addDebugInfo(SEM::Value::InterfaceMethodObject(std::move(functionRef), std::move(value)), location);
				} else {
					return addDebugInfo(SEM::Value::MethodObject(std::move(functionRef), std::move(value)), location);
				}
			} else {
				const bool isTemplated = true;
				const auto functionType = methodElement.createFunctionType(isTemplated);
				auto functionRef = addDebugInfo(SEM::Value::TemplateFunctionRef(type, methodName, functionType), location);
				return addDebugInfo(SEM::Value::MethodObject(std::move(functionRef), std::move(value)), location);
			}
		}
		
		SEM::Value CallValue(Context& context, SEM::Value rawValue, HeapArray<SEM::Value> args, const Debug::SourceLocation& location) {
			auto value = tryDissolveValue(context, derefValue(std::move(rawValue)), location);
			
			if (getDerefType(value.type())->isStaticRef()) {
				return CallValue(context, GetStaticMethod(context, std::move(value), context.getCString("create"), location), std::move(args), location);
			}
			
			if (!isCallableType(value.type())) {
				throw ErrorException(makeString("Can't call value '%s' that isn't a function or a method at position %s.",
					value.toString().c_str(), location.toString().c_str()));
			}
			
			const auto functionType = value.type()->getCallableFunctionType();
			const auto& typeList = functionType->getFunctionParameterTypes();
			
			if (functionType->isFunctionVarArg()) {
				if (args.size() < typeList.size()) {
					throw ErrorException(makeString("Var Arg Function [%s] called with %llu "
						"parameters; expected at least %llu at position %s.",
						value.toString().c_str(),
						(unsigned long long) args.size(),
						(unsigned long long) typeList.size(),
						location.toString().c_str()));
				}
			} else {
				if (args.size() != typeList.size()) {
					throw ErrorException(makeString("Function [%s] called with %llu "
						"parameters; expected %llu at position %s.",
						value.toString().c_str(),
						(unsigned long long) args.size(),
						(unsigned long long) typeList.size(),
						location.toString().c_str()));
				}
			}
			
			return addDebugInfo(SEM::Value::Call(std::move(value), CastFunctionArguments(context, std::move(args), typeList, location)), location);
		}
		
		bool checkCapability(Context& context, const SEM::Type* const rawType, const String& capability, SEM::TypeArray templateArgs) {
			const auto type = rawType->resolveAliases();
			if (!type->isObject() && !type->isTemplateVar()) {
				return false;
			}
			
			const Optional<bool> previousResult = context.getCapability(type, capability);
			if (previousResult) {
				return *previousResult;
			}
			
			for (auto& arg: templateArgs) {
				arg = arg->resolveAliases();
			}
			
			const auto requireType = getBuiltInType(context, capability, std::move(templateArgs))->resolveAliases();
			
			const auto sourceMethodSet = getTypeMethodSet(context, type);
			const auto requireMethodSet = getTypeMethodSet(context, requireType);
			
			const bool result = methodSetSatisfiesRequirement(sourceMethodSet, requireMethodSet);
			context.setCapability(type, capability, result);
			return result;
		}
		
		bool supportsNullConstruction(Context& context, const SEM::Type* type) {
			return checkCapability(context, type, context.getCString("null_constructible"), {});
		}
		
		bool supportsImplicitCast(Context& context, const SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
				case SEM::Type::TEMPLATEVAR:
					return false;
					
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find(context.getCString("implicitcast"));
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					
					// Conservatively assume method is not const if result is undetermined.
					const bool isConstMethodDefault = false;
					
					if (!evaluatePredicateWithDefault(context, function->constPredicate(), type->generateTemplateVarMap(), isConstMethodDefault)) return false;
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
		
		bool supportsImplicitCopy(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("implicit_copyable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptImplicitCopy(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("noexcept_implicit_copyable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsExplicitCopy(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("copyable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptExplicitCopy(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("noexcept_copyable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsCompare(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("comparable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptCompare(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("noexcept_comparable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsMove(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("movable"), {});
		}
		
		bool supportsDissolve(Context& context, const SEM::Type* const type) {
			assert(type->isLval());
			return checkCapability(context, type, context.getCString("dissolvable"), { type->lvalTarget() }) ||
				checkCapability(context, type, context.getCString("const_dissolvable"), { type->lvalTarget() });
		}
		
	}
	
}


