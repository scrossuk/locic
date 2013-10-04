#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>
#include <Locic/SemanticAnalysis/Lval.hpp>
#include <Locic/SemanticAnalysis/TypeProperties.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Type* makeValueLvalType(Context& context, bool isLvalMutable, SEM::Type* valueType) {
			SEM::TypeInstance* valueLvalTypeInstance = context.getBuiltInType("value_lval");
			SEM::Type* valueLvalType = SEM::Type::Object(isLvalMutable, valueLvalTypeInstance, std::vector<SEM::Type*>(1, valueType));
			return valueLvalType;
		}
		
		// If the type 'T' given isn't an lval,
		// return the SEM tree for 'lval value_lval<T>'.
		SEM::Type* makeLvalType(Context& context, bool usesCustomLval, bool isLvalMutable, SEM::Type* valueType) {
			return usesCustomLval ? valueType : makeValueLvalType(context, isLvalMutable, valueType);
		}
		
		bool canDissolveValue(SEM::Value* value) {
			// Dereference any number of references.
			while (value->type()->isReference()) {
				value = SEM::Value::DerefReference(value);
			}
			
			SEM::Type* type = value->type();
			
			if (!type->isObject()) {
				return false;
			}
			
			if (!type->getObjectType()->hasProperty("dissolve")) {
				return false;
			}
			
			return true;
		}
		
		SEM::Value* dissolveLval(SEM::Value* value) {
			// Dereference any number of references.
			while (value->type()->isReference()) {
				value = SEM::Value::DerefReference(value);
			}
			
			SEM::Type* type = value->type();
			
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


