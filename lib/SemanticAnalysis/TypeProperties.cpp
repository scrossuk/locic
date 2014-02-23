#include <assert.h>

#include <string>
#include <vector>

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/MethodPattern.hpp>
#include <locic/SemanticAnalysis/Node.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Value* CallPropertyFunction(SEM::Type* type, const std::string& propertyName, const std::vector<SEM::Value*>& args) {
			assert(type->isObject());
			assert(type->getObjectType()->hasProperty(propertyName));
			
			const auto function = type->getObjectType()->getProperty(propertyName);
			const auto functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			return SEM::Value::FunctionCall(functionRef, args);
		}
		
		SEM::Value* CallPropertyMethod(SEM::Value* value, const std::string& propertyName, const std::vector<SEM::Value*>& args) {
			const auto type = getDerefType(value->type());
			assert(type->isObject());
			assert(type->getObjectType()->hasProperty(propertyName));
			
			const auto function = type->getObjectType()->getProperty(propertyName);
			
			const auto functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			const auto methodRef = SEM::Value::MethodObject(functionRef, derefValue(value));
			return SEM::Value::MethodCall(methodRef, args);
		}
	}
	
}


