#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/MethodPattern.hpp>
#include <locic/SemanticAnalysis/Node.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Value* CallPropertyFunction(SEM::Type* type, const std::string& propertyName, const std::vector<SEM::Value*>& args) {
			if (!type->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Cannot call property '%s' on non-object type '%s'.",
					propertyName.c_str(), type->toString().c_str()));
			}
			
			const auto typeInstance = type->isObject() ? type->getObjectType() : type->getTemplateVar()->specTypeInstance();
			
			if (!typeInstance->hasProperty(propertyName)) {
				throw ErrorException(makeString("Type '%s' does not support property '%s'.",
					typeInstance->refToString().c_str(), propertyName.c_str()));
			}
			
			const auto function = typeInstance->getProperty(propertyName);
			const auto functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			return SEM::Value::FunctionCall(functionRef, args);
		}
		
		SEM::Value* CallPropertyMethod(SEM::Value* value, const std::string& propertyName, const std::vector<SEM::Value*>& args) {
			const auto type = getDerefType(value->type());
			
			if (!type->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Cannot call property '%s' on non-object type '%s'.",
					propertyName.c_str(), type->toString().c_str()));
			}
			
			const auto typeInstance = type->isObject() ? type->getObjectType() : type->getTemplateVar()->specTypeInstance();
			
			if (!typeInstance->hasProperty(propertyName)) {
				throw ErrorException(makeString("Type '%s' does not support property '%s'.",
					typeInstance->refToString().c_str(), propertyName.c_str()));
			}
			
			const auto function = typeInstance->getProperty(propertyName);
			
			const auto functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			const auto methodRef = SEM::Value::MethodObject(functionRef, derefValue(value));
			return SEM::Value::MethodCall(methodRef, args);
		}
		
		bool supportsImplicitCopy(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
					// Built-in types can be copied implicitly.
					return true;
					
				case SEM::Type::OBJECT:
					// Named types must have a method for implicit copying.
					return type->getObjectType()->hasProperty("implicitCopy");
					
				case SEM::Type::TEMPLATEVAR:
					return type->getTemplateVar()->specTypeInstance()->hasProperty("implicitCopy");
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
	}
	
}


