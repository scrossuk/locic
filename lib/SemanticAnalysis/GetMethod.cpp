#include <cassert>

#include <locic/AST/MethodSet.hpp>
#include <locic/AST/Type.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			AST::Value addDebugInfo(AST::Value value, const Debug::SourceLocation& location) {
				Debug::ValueInfo valueInfo;
				valueInfo.location = location;
				value.setDebugInfo(valueInfo);
				return value;
			}
			
		}
		
		AST::FunctionType simplifyFunctionType(Context& context, const AST::FunctionType oldFunctionType) {
			const bool isVarArg = oldFunctionType.attributes().isVarArg();
			const bool isMethod = oldFunctionType.attributes().isMethod();
			const bool isTemplated = oldFunctionType.attributes().isTemplated();
			auto noexceptPredicate = reducePredicate(context, oldFunctionType.attributes().noExceptPredicate().copy());
			const auto returnType = oldFunctionType.returnType();
			const auto& argTypes = oldFunctionType.parameterTypes();
			
			return AST::FunctionType(AST::FunctionAttributes(isVarArg, isMethod, isTemplated, std::move(noexceptPredicate)), returnType, argTypes.copy());
		}
		
		Diag
		CannotFindStaticMethodDiag(const String name, const AST::Type* const type) {
			return Error("cannot find static method '%s' for type '%s'",
			             name.c_str(), type->toDiagString().c_str());
		}
		
		Diag
		CannotCallNonStaticMethodDiag(const String name, const AST::Type* const type) {
			return Error("cannot call non-static method '%s' for type '%s'",
			             name.c_str(), type->toDiagString().c_str());
		}
		
		AST::Value GetStaticMethod(Context& context, AST::Value rawValue, const String& methodName, const Debug::SourceLocation& location) {
			auto value = derefOrBindValue(context, std::move(rawValue));
			assert(value.type()->isRef());
			assert(value.type()->refTarget()->isTypename());
			const auto targetType = value.type()->refTarget()->typenameTarget()->resolveAliases();
			assert(targetType->isObjectOrTemplateVar());
			
			const auto methodSet = getTypeMethodSet(context, targetType);
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = methodSet->find(canonicalMethodName);
			
			if (methodIterator == methodSet->end()) {
				context.issueDiag(CannotFindStaticMethodDiag(methodName, targetType),
				                  location);
				return AST::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
			}
			
			const auto& methodElement = methodIterator->second;
			
			if (!methodElement.isStatic()) {
				context.issueDiag(CannotCallNonStaticMethodDiag(methodName, targetType),
				                  location);
			}
			
			auto& typeBuilder = context.typeBuilder();
			
			if (targetType->isObject()) {
				// Get the actual function so we can refer to it.
				const auto& function = targetType->getObjectType()->getFunction(canonicalMethodName);
				const auto functionTypeTemplateMap = targetType->generateTemplateVarMap();
				
				const auto functionType = simplifyFunctionType(context, function.type().substitute(functionTypeTemplateMap,
				                                                                                   /*selfconst=*/AST::Predicate::SelfConst()));
				const auto functionRefType = typeBuilder.getFunctionPointerType(functionType);
				
				auto functionRef = addDebugInfo(AST::Value::FunctionRef(targetType, function, {}, functionRefType), location);
				
				if (targetType->isInterface()) {
					const auto interfaceMethodType = typeBuilder.getStaticInterfaceMethodType(functionType);
					return addDebugInfo(AST::Value::StaticInterfaceMethodObject(std::move(functionRef), std::move(value), interfaceMethodType), location);
				} else {
					return functionRef;
				}
			} else {
				const bool isTemplated = true;
				const auto functionType = typeBuilder.getFunctionPointerType(methodElement.createFunctionType(isTemplated));
				return addDebugInfo(AST::Value::TemplateFunctionRef(targetType, methodName, functionType), location);
			}
		}
		
		AST::Value GetMethod(Context& context, AST::Value rawValue, const String& methodName, const Debug::SourceLocation& location) {
			return GetTemplatedMethod(context, std::move(rawValue), methodName, {}, location);
		}
		
		AST::Value GetTemplatedMethod(Context& context, AST::Value rawValue, const String& methodName, AST::ValueArray templateArguments, const Debug::SourceLocation& location) {
			auto value = derefOrBindValue(context, std::move(rawValue));
			const auto type = getDerefType(value.type())->resolveAliases();
			return GetTemplatedMethodWithoutResolution(context, std::move(value), type, methodName, std::move(templateArguments), location);
		}
		
		// Gets the method without dissolving or derefencing the object.
		AST::Value GetSpecialMethod(Context& context, AST::Value value, const String& methodName, const Debug::SourceLocation& location) {
			assert(value.type()->isRef());
			const auto type = value.type()->refTarget()->resolveAliases();
			return GetMethodWithoutResolution(context, std::move(value), type, methodName, location);
		}
		
		AST::Value GetMethodWithoutResolution(Context& context, AST::Value value, const AST::Type* type, const String& methodName, const Debug::SourceLocation& location) {
			return GetTemplatedMethodWithoutResolution(context, std::move(value), type, methodName, {}, location);
		}
		
		Diag
		CannotFindMethodDiag(const String name, const AST::Type* const type) {
			return Error("cannot find method '%s' for type '%s'",
			             name.c_str(), type->toDiagString().c_str());
		}
		
		Diag
		CannotAccessStaticMethodDiag(const String name, const AST::Type* const type) {
			return Error("cannot access static method '%s' for value of type '%s'",
			             name.c_str(), type->toDiagString().c_str());
		}
		
		Diag
		InvalidMethodTemplateArgCountDiag(const String name,
		                                  const size_t argsExpected,
		                                  const size_t argsGiven) {
			return Error("incorrect number of template arguments provided "
			             "for method '%s'; %zu were required, but %zu "
			             "were provided", name.c_str(), argsExpected,
			             argsGiven);
		}
		
		Diag
		InvalidMethodTemplateArgDiag(const AST::Type* const type, const String varName,
		                             const String methodName) {
			return Error("invalid type '%s' passed as template parameter "
			             "'%s' for method '%s'", type->toDiagString().c_str(),
			             varName.c_str(), methodName.c_str());
		}
		
		Diag
		CannotReferToMutatorMethodFromConstDiag(const String name,
		                                        const AST::Type* const type) {
			return Error("cannot refer to mutator method '%s' from "
			             "const object of type '%s'", name.c_str(),
			             type->toDiagString().c_str());
		}
		
		Diag
		TemplateArgsDoNotSatisfyMethodRequirePredicateDiag(const AST::Predicate& requirePredicate,
		                                                   const String& name) {
			return Error("template arguments do not satisfy require predicate "
			             "'%s' of method '%s'", requirePredicate.toString().c_str(),
			             name.c_str());
		}
		
		AST::Value GetTemplatedMethodWithoutResolution(Context& context, AST::Value value, const AST::Type* const type, const String& methodName, AST::ValueArray templateArguments, const Debug::SourceLocation& location) {
			assert(value.type()->isRef());
			assert(type->isObjectOrTemplateVar());
			
			const auto methodSet = getTypeMethodSet(context, type);
			const auto& objectConstPredicate = methodSet->constPredicate();
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = methodSet->find(canonicalMethodName);
			
			if (methodIterator == methodSet->end()) {
				context.issueDiag(CannotFindMethodDiag(methodName, type),
				                  location);
				return AST::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
			}
			
			const auto& methodElement = methodIterator->second;
			if (methodElement.isStatic()) {
				context.issueDiag(CannotAccessStaticMethodDiag(methodName, type),
				                  location);
			}
			
			auto templateVariableAssignments = type->generateTemplateVarMap();
			
			const auto& templateVariables = methodElement.templateVariables();
			if (templateVariables.size() != templateArguments.size()) {
				// Try to apply some basic deduction...
				if (templateVariables.size() == 1 && templateArguments.size() == 0 &&
				    methodElement.constPredicate().isVariable() &&
				    methodElement.constPredicate().variableTemplateVar() == templateVariables[0]
				) {
					const auto boolType = context.typeBuilder().getBoolType();
					templateArguments.push_back(AST::Value::PredicateExpr(objectConstPredicate.copy(), boolType));
				} else {
					context.issueDiag(InvalidMethodTemplateArgCountDiag(methodName,
					                                                    templateVariables.size(),
					                                                    templateArguments.size()),
					                  location);
				}
			}
			
			// Add method template variable => argument mapping.
			for (size_t i = 0; i < std::min(templateVariables.size(), templateArguments.size()); i++) {
				const auto templateVariable = templateVariables.at(i);
				const auto& templateValue = templateArguments.at(i);
				
				if (templateValue.isTypeRef()) {
					const auto templateTypeValue = templateValue.typeRefType()->resolveAliases();
					
					if (!templateTypeValue->isObjectOrTemplateVar() || templateTypeValue->isInterface()) {
						context.issueDiag(InvalidMethodTemplateArgDiag(templateTypeValue,
						                                               templateVariable->fullName().last(),
						                                               methodName),
						                  location);
					}
					
					templateVariableAssignments.insert(std::make_pair(templateVariable, AST::Value::TypeRef(templateTypeValue, templateValue.type())));
				} else {
					templateVariableAssignments.insert(std::make_pair(templateVariable, templateValue.copy()));
				}
			}
			
			const auto methodConstPredicate = methodElement.constPredicate().substitute(templateVariableAssignments,
			                                                                            /*selfconst=*/AST::Predicate::SelfConst());
			
			if (!objectConstPredicate.implies(methodConstPredicate)) {
				context.issueDiag(CannotReferToMutatorMethodFromConstDiag(methodName, type),
				                  location);
			}
			
			// Now check the template arguments satisfy the requires predicate.
			const auto& requirePredicate = methodElement.requirePredicate();
			
			auto result = evaluatePredicate(context, requirePredicate, templateVariableAssignments);
			if (result.failed()) {
				const auto substitutedRequirePredicate = requirePredicate.substitute(templateVariableAssignments,
				                                                                     /*selfconst=*/AST::Predicate::SelfConst());
				context.issueDiag(TemplateArgsDoNotSatisfyMethodRequirePredicateDiag(substitutedRequirePredicate,
				                                                                     methodName),
				                  location, std::move(result));
			}
			
			auto& typeBuilder = context.typeBuilder();
			
			const auto function = type->isObject() ? &(type->getObjectType()->getFunction(canonicalMethodName)) : nullptr;
			if (function != nullptr) {
				const auto substitutedType = function->type().substitute(templateVariableAssignments,
				                                                         /*selfconst=*/objectConstPredicate);
				const auto functionType = simplifyFunctionType(context, substitutedType);
				const auto functionRefType = typeBuilder.getFunctionPointerType(functionType);
				
				auto functionRef = addDebugInfo(AST::Value::FunctionRef(type, *function, std::move(templateArguments), functionRefType), location);
				if (methodElement.isStatic()) {
					return functionRef;
				}
				
				if (type->isInterface()) {
					const auto interfaceMethodType = typeBuilder.getInterfaceMethodType(functionType);
					return addDebugInfo(AST::Value::InterfaceMethodObject(std::move(functionRef), std::move(value), interfaceMethodType), location);
				} else {
					const auto methodType = typeBuilder.getMethodType(functionType);
					return addDebugInfo(AST::Value::MethodObject(std::move(functionRef), std::move(value), methodType), location);
				}
			} else {
				const bool isTemplated = true;
				const auto functionType = methodElement.createFunctionType(isTemplated);
				const auto substitutedType = functionType.substitute(templateVariableAssignments,
				                                                     /*selfconst=*/objectConstPredicate);
				const auto functionRefType = typeBuilder.getFunctionPointerType(substitutedType);
				auto functionRef = addDebugInfo(AST::Value::TemplateFunctionRef(type, methodName, functionRefType), location);
				if (methodElement.isStatic()) {
					return functionRef;
				}
				
				const auto methodType = typeBuilder.getMethodType(substitutedType);
				return addDebugInfo(AST::Value::MethodObject(std::move(functionRef), std::move(value), methodType), location);
			}
		}
		
	}
	
}


