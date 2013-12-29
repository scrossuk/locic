#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Type* makeValueLvalType(Context& context, bool isLvalConst, SEM::Type* valueType) {
			SEM::TypeInstance* valueLvalTypeInstance = context.getBuiltInType("value_lval");
			SEM::Type* valueLvalType = SEM::Type::Object(valueLvalTypeInstance, std::vector<SEM::Type*>(1, valueType))->createLvalType(valueType);
			return isLvalConst ? valueLvalType->createConstType() : valueLvalType;
		}
		
		SEM::Type* makeLvalType(Context& context, bool isLvalConst, SEM::Type* valueType) {
			return getDerefType(valueType)->isLval() ? valueType : makeValueLvalType(context, isLvalConst, valueType);
		}
		
		size_t getLvalCount(SEM::Type* type) {
			type = getDerefType(type);
			
			size_t count = 0;
			while (type->isLval()) {
				count++;
				type = getDerefType(type->lvalTarget());
			}
			return count;
		}
		
		bool canDissolveValue(SEM::Value* value) {
			auto type = getDerefType(value->type());
			return type->isLval() && type->isObject() && type->getObjectType()->hasProperty("dissolve");
		}
		
		SEM::Value* dissolveLval(SEM::Value* value) {
			auto type = getDerefType(value->type());
			
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
			
			return CallProperty(derefValue(value), "dissolve", std::vector<SEM::Value*>());
		}
		
		SEM::Value* tryDissolveValue(SEM::Value* value) {
			return canDissolveValue(value) ? dissolveLval(value) : value;
		}
		
	}
	
}


