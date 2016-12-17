#include <cassert>

#include <locic/Frontend/OptionalDiag.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
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
		
		class CannotFindStaticMethodDiag: public Error {
		public:
			CannotFindStaticMethodDiag(const String name, const SEM::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot find static method '%s' for type '%s'",
				                  name_.c_str(), typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		class CannotCallNonStaticMethodDiag: public Error {
		public:
			CannotCallNonStaticMethodDiag(const String name, const SEM::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot call non-static method '%s' for type '%s'",
				                  name_.c_str(), typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		SEM::Value GetStaticMethod(Context& context, SEM::Value rawValue, const String& methodName, const Debug::SourceLocation& location) {
			auto value = derefOrBindValue(context, std::move(rawValue));
			assert(value.type()->isRef() && value.type()->isBuiltInReference());
			assert(value.type()->refTarget()->isStaticRef());
			const auto targetType = value.type()->refTarget()->staticRefTarget()->resolveAliases();
			assert(targetType->isObjectOrTemplateVar());
			
			const auto methodSet = getTypeMethodSet(context, targetType);
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = methodSet->find(canonicalMethodName);
			
			if (methodIterator == methodSet->end()) {
				context.issueDiag(CannotFindStaticMethodDiag(methodName, targetType),
				                  location);
				return SEM::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
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
				
				const auto functionType = simplifyFunctionType(context, function.type().substitute(functionTypeTemplateMap));
				const auto functionRefType = typeBuilder.getFunctionPointerType(functionType);
				
				auto functionRef = addDebugInfo(SEM::Value::FunctionRef(targetType, function, {}, functionRefType), location);
				
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
		
		class CannotFindMethodDiag: public Error {
		public:
			CannotFindMethodDiag(const String name, const SEM::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot find method '%s' for type '%s'",
				                  name_.c_str(), typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		class CannotAccessStaticMethodDiag: public Error {
		public:
			CannotAccessStaticMethodDiag(const String name, const SEM::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot access static method '%s' for value of type '%s'",
				                  name_.c_str(), typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		class InvalidMethodTemplateArgCountDiag: public Error {
		public:
			InvalidMethodTemplateArgCountDiag(const String name,
			                                  size_t argsExpected,
			                                  size_t argsGiven)
			: name_(name), argsExpected_(argsExpected), argsGiven_(argsGiven) { }
			
			std::string toString() const {
				return makeString("incorrect number of template arguments provided "
				                  "for method '%s'; %zu were required, but %zu "
						  "were provided", name_.c_str(), argsExpected_,
				                  argsGiven_);
			}
			
		private:
			String name_;
			size_t argsExpected_;
			size_t argsGiven_;
			
		};
		
		class InvalidMethodTemplateArgDiag: public Error {
		public:
			InvalidMethodTemplateArgDiag(const SEM::Type* type, const String varName,
			                             const String methodName)
			: typeString_(type->toDiagString()), varName_(varName),
			methodName_(methodName) { }
			
			std::string toString() const {
				return makeString("invalid type '%s' passed as template parameter "
				                  "'%s' for method '%s'", typeString_.c_str(),
				                  varName_.c_str(), methodName_.c_str());
			}
			
		private:
			std::string typeString_;
			String varName_;
			String methodName_;
			
		};
		
		class CannotReferToMutatorMethodFromConstDiag: public Error {
		public:
			CannotReferToMutatorMethodFromConstDiag(const String name,
			                                        const SEM::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot refer to mutator method '%s' from "
				                  "const object of type '%s'", name_.c_str(),
				                  typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		class TemplateArgsDoNotSatisfyMethodRequirePredicateDiag : public Error {
		public:
			TemplateArgsDoNotSatisfyMethodRequirePredicateDiag(const SEM::Predicate& requirePredicate,
			                                                   const String& name)
			: requirePredicateString_(requirePredicate.toString()), name_(name) { }

			std::string toString() const {
				return makeString("template arguments do not satisfy require predicate "
				                  "'%s' of method '%s'", requirePredicateString_.c_str(),
				                  name_.c_str());
			}

		private:
			std::string requirePredicateString_;
			String name_;
			
		};
		
		SEM::Value GetTemplatedMethodWithoutResolution(Context& context, SEM::Value value, const SEM::Type* const type, const String& methodName, SEM::ValueArray templateArguments, const Debug::SourceLocation& location) {
			assert(value.type()->isRef() && value.type()->isBuiltInReference());
			assert(type->isObjectOrTemplateVar());
			
			const auto methodSet = getTypeMethodSet(context, type);
			const auto& objectConstPredicate = methodSet->constPredicate();
			
			const auto canonicalMethodName = CanonicalizeMethodName(methodName);
			const auto methodIterator = methodSet->find(canonicalMethodName);
			
			if (methodIterator == methodSet->end()) {
				context.issueDiag(CannotFindMethodDiag(methodName, type),
				                  location);
				return SEM::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
			}
			
			const auto& methodElement = methodIterator->second;
			if (methodElement.isStatic()) {
				context.issueDiag(CannotAccessStaticMethodDiag(methodName, type),
				                  location);
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
						context.issueDiag(InvalidMethodTemplateArgCountDiag(function->fullName().last(),
						                                                    templateVariables.size(),
						                                                    templateArguments.size()),
						                  location);
					}
				}
				
				// Add function template variable => argument mapping.
				for (size_t i = 0; i < std::min(templateVariables.size(), templateArguments.size()); i++) {
					const auto templateVariable = templateVariables.at(i);
					const auto& templateValue = templateArguments.at(i);
					
					if (templateValue.isTypeRef()) {
						const auto templateTypeValue = templateValue.typeRefType()->resolveAliases();
						
						if (!templateTypeValue->isObjectOrTemplateVar() || templateTypeValue->isInterface()) {
							context.issueDiag(InvalidMethodTemplateArgDiag(templateTypeValue,
							                                               templateVariable->name().last(),
							                                               function->fullName().last()),
							                  location);
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
				context.issueDiag(CannotReferToMutatorMethodFromConstDiag(methodName, type),
				                  location);
			}
			
			// Now check the template arguments satisfy the requires predicate.
			const auto& requirePredicate = methodElement.requirePredicate();
			
			auto result = evaluatePredicate(context, requirePredicate, templateVariableAssignments);
			if (!result) {
				const auto substitutedRequirePredicate = requirePredicate.substitute(templateVariableAssignments);
				context.issueDiag(TemplateArgsDoNotSatisfyMethodRequirePredicateDiag(substitutedRequirePredicate,
				                                                                     methodName),
				                  location, std::move(result));
			}
			
			auto& typeBuilder = context.typeBuilder();
			
			if (function != nullptr) {
				const auto functionType = simplifyFunctionType(context, function->type().substitute(templateVariableAssignments));
				const auto functionRefType = typeBuilder.getFunctionPointerType(functionType);
				
				auto functionRef = addDebugInfo(SEM::Value::FunctionRef(type, *function, std::move(templateArguments), functionRefType), location);
				if (methodElement.isStatic()) {
					return functionRef;
				}
				
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
				if (methodElement.isStatic()) {
					return functionRef;
				}
				
				const auto methodType = typeBuilder.getMethodType(functionType);
				return addDebugInfo(SEM::Value::MethodObject(std::move(functionRef), std::move(value), methodType), location);
			}
		}
		
	}
	
}


