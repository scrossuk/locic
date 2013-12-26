#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/MethodPattern.hpp>
#include <locic/SemanticAnalysis/Node.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Value* CallProperty(SEM::Value* value, const std::string& propertyName, const std::vector<SEM::Value*>& args) {
			SEM::Type* type = value->type();
			assert(type->isObject());
			assert(type->getObjectType()->hasProperty(propertyName));
			
			SEM::Function* function = type->getObjectType()->getProperty(propertyName);
			
			SEM::Value* functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			SEM::Value* methodRef = SEM::Value::MethodObject(functionRef, value);
			
			return SEM::Value::MethodCall(methodRef, args);
		}
	}
	
}


