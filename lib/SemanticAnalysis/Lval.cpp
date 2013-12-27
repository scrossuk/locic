#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Type* makeValueLvalType(Context& context, bool isLvalConst, SEM::Type* valueType) {
			SEM::TypeInstance* valueLvalTypeInstance = context.getBuiltInType("value_lval");
			SEM::Type* valueLvalType = SEM::Type::Object(valueLvalTypeInstance, std::vector<SEM::Type*>(1, valueType))->createLvalType();
			return isLvalConst ? valueLvalType->createConstType() : valueLvalType;
		}
		
		bool canDissolveValue(SEM::Value* value) {
			// Dereference any number of references.
			while (value->type()->isReference()) {
				value = SEM::Value::DerefReference(value);
			}
			
			auto type = value->type();
			return type->isLval() && type->isObject() && type->getObjectType()->hasProperty("dissolve");
		}
		
		SEM::Value* dissolveLval(SEM::Value* value) {
			// Dereference any number of references.
			while (value->type()->isReference()) {
				value = SEM::Value::DerefReference(value);
			}
			
			auto type = value->type();
			
			if (!type->isLval()) {
				throw TodoException(makeString("Type '%s' is not an lval and hence it cannot be dissolved.",
					type->toString().c_str()));
			}
			
			if (!type->isObject()) {
				throw TodoException(makeString("Type '%s' is not an object type and hence it cannot be dissolved.",
					type->toString().c_str()));
			}
			
			if (!type->getObjectType()->hasProperty("dissolve")) {
				throw TodoException(makeString("Type '%s' does not support 'dissolve'.",
					type->toString().c_str()));
			}
			
			return CallProperty(value, "dissolve", std::vector<SEM::Value*>());
		}
		
		SEM::Value* tryDissolveValue(SEM::Value* value) {
			return canDissolveValue(value) ? dissolveLval(value) : value;
		}
		
	}
	
}


