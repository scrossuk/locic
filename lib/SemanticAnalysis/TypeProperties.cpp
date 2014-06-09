#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			std::vector<SEM::Value*> CastFunctionArguments(const std::vector<SEM::Value*>& arguments, const std::vector<SEM::Type*>& types, const Debug::SourceLocation& location) {
				std::vector<SEM::Value*> castValues;
				
				for (size_t i = 0; i < arguments.size(); i++) {
					const auto argumentValue = arguments.at(i);
					
					// Cast arguments to the function type's corresponding
					// argument type; var-arg arguments should be cast to
					// one of the allowed types (since there's no specific
					// destination type).
					const auto castArgumentValue = (i < types.size()) ?
						ImplicitCast(argumentValue, types.at(i), location) :
						VarArgCast(argumentValue, location);
					
					castValues.push_back(castArgumentValue);
				}
				
				return castValues;
			}
			
			bool isCallableType(SEM::Type* type) {
				switch (type->kind()) {
					case SEM::Type::FUNCTION:
					case SEM::Type::METHOD:
					case SEM::Type::INTERFACEMETHOD:
						return true;
					default:
						return false;
				}
			}
			
			SEM::Type* getFunctionType(SEM::Type* type) {
				switch (type->kind()) {
					case SEM::Type::FUNCTION:
						return type;
					case SEM::Type::METHOD:
						return type->getMethodFunctionType();
					case SEM::Type::INTERFACEMETHOD:
						return type->getInterfaceMethodFunctionType();
					default:
						throw std::runtime_error("Cannot get function type of non-function value.");
				}
			}
			
			SEM::Value* dissolveObject(SEM::Value* object, const std::string& methodName, const Debug::SourceLocation& location) {
				if (methodName != "address" && methodName != "assign" && methodName != "dissolve" && methodName != "move") {
					return tryDissolveValue(object, location);
				} else {
					return object;
				}
			}
			
		}
		
		SEM::Value* GetStaticMethod(SEM::Type* type, const std::string& methodName, const Debug::SourceLocation& location) {
			if (!type->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Cannot get static method '%s' for non-object type '%s' at position %s.",
					methodName.c_str(), type->toString().c_str(), location.toString().c_str()));
			}
			
			const auto typeInstance = type->isObject() ? type->getObjectType() : type->getTemplateVar()->specTypeInstance();
			
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
			
			return SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
		}
		
		SEM::Value* GetMethod(SEM::Value* rawValue, const std::string& methodName, const Debug::SourceLocation& location) {
			const auto value = dissolveObject(rawValue, methodName, location);
			const auto type = getDerefType(value->type());
			
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
				throw ErrorException(makeString("Cannot access static method '%s' for value '%s' of type '%s' at position %s.",
					methodName.c_str(), value->toString().c_str(),
					typeInstance->refToString().c_str(), location.toString().c_str()));
			}
			
			if (type->isConst() && !function->isConstMethod()) {
				throw ErrorException(makeString("Cannot refer to mutator method '%s' from const object '%s' of type '%s' at position %s.",
					function->name().toString().c_str(), value->toString().c_str(),
					type->toString().c_str(), location.toString().c_str()));
			}
			
			const auto functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			
			if (typeInstance->isInterface()) {
				return SEM::Value::InterfaceMethodObject(functionRef, derefValue(value));
			} else {
				return SEM::Value::MethodObject(functionRef, derefValue(value));
			}
		}
		
		SEM::Value* CallValue(SEM::Value* value, const std::vector<SEM::Value*>& args, const Debug::SourceLocation& location) {
			if (!isCallableType(value->type())) {
				throw ErrorException(makeString("Can't call value '%s' that isn't a function or a method at position %s.",
					value->toString().c_str(), location.toString().c_str()));
			}
			
			const auto functionType = getFunctionType(value->type());
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
			
			return SEM::Value::FunctionCall(value, CastFunctionArguments(args, typeList, location));
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
		
		bool supportsPrimitiveCast(SEM::Type* type, const std::string& primitiveName) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
					return false;
					
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto canonicalMethodName = CanonicalizeMethodName(primitiveName + "_cast");
					const auto methodIterator = typeInstance->functions().find(canonicalMethodName);
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->isMethod()) return false;
					if (!function->isStaticMethod()) return false;
					if (function->isConstMethod()) return false;
					if (function->parameters().size() != 1) return false;
					
					const auto operandType = function->parameters().at(0)->constructType();
					if (!operandType->isObject()) return false;
					
					if (!operandType->getObjectType()->isPrimitive()) return false;
					if (operandType->getObjectType()->name().last() != primitiveName + "_t") return false;
					
					return true;
				}
				
				case SEM::Type::TEMPLATEVAR:
					return supportsPrimitiveCast(type->getTemplateVar()->specTypeInstance()->selfType(), primitiveName);
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
		bool supportsImplicitCopy(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
					// Built-in types can be copied implicitly.
					return true;
					
				case SEM::Type::OBJECT: {
					// Named types must have a method for implicit copying.
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find("implicitcopy");
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					if (!function->isConstMethod()) return false;
					if (!function->parameters().empty()) return false;
					
					return true;
				}
				
				case SEM::Type::TEMPLATEVAR:
					return supportsImplicitCopy(type->getTemplateVar()->specTypeInstance()->selfType());
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
		bool supportsCompare(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
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
					
					const auto firstArg = function->parameters().at(0);
					if (*(firstArg->constructType()) != *type) return false;
					
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


