#include <assert.h>

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
				throw TodoException(makeString("Cannot call property '%s' on non-object type '%s'.",
					propertyName.c_str(), type->toString().c_str()));
			}
			
			const auto typeInstance = type->isObject() ? type->getObjectType() : type->getTemplateVar()->specTypeInstance();
			
			if (!typeInstance->hasProperty(propertyName)) {
				throw TodoException(makeString("Type '%s' does not support property '%s'.",
					typeInstance->refToString().c_str(), propertyName.c_str()));
			}
			
			const auto function = typeInstance->getProperty(propertyName);
			const auto functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			return SEM::Value::FunctionCall(functionRef, args);
		}
		
		SEM::Value* CallPropertyMethod(SEM::Value* value, const std::string& propertyName, const std::vector<SEM::Value*>& args) {
			const auto type = getDerefType(value->type());
			
			if (!type->isObjectOrTemplateVar()) {
				throw TodoException(makeString("Cannot call property '%s' on non-object type '%s'.",
					propertyName.c_str(), type->toString().c_str()));
			}
			
			const auto typeInstance = type->isObject() ? type->getObjectType() : type->getTemplateVar()->specTypeInstance();
			
			if (!typeInstance->hasProperty(propertyName)) {
				throw TodoException(makeString("Type '%s' does not support property '%s'.",
					typeInstance->refToString().c_str(), propertyName.c_str()));
			}
			
			const auto function = typeInstance->getProperty(propertyName);
			
			const auto functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			const auto methodRef = SEM::Value::MethodObject(functionRef, derefValue(value));
			return SEM::Value::MethodCall(methodRef, args);
		}
	}
	
}


