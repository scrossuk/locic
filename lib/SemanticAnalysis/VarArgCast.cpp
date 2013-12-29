#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		bool isValidVarArgType(SEM::Type* type) {
			if (!type->isObject()) return false;
			if (!type->getObjectType()->isPrimitive()) return false;
			if (type->isLval() || type->isRef()) return false;
			
			const std::string name = type->getObjectType()->name().first();
			
			// TODO: find a better (cleaner) way to do this...
			if (name == "char") return true;
			if (name == "short") return true;
			if (name == "int") return true;
			if (name == "long") return true;
			if (name == "float") return true;
			if (name == "double") return true;
			if (name == "ptr") return true;
			
			return false;
		}
		
		static inline SEM::Value* VarArgCastSearch(SEM::Value* value) {
			if (isValidVarArgType(value->type())) {
				// Already a valid var arg type.
				return value;
			}
			
			auto derefType = getDerefType(value->type());
			assert(!derefType->isRef());
			
			// TODO: remove 'canDissolveValue()', because 'isLval()' should be enough.
			if (derefType->isLval() && canDissolveValue(derefValue(value))) {
				// Dissolve lval.
				auto dissolvedValue = dissolveLval(derefValue(value));
				
				// See if this results in
				// a valid var arg value.
				auto result = VarArgCastSearch(dissolvedValue);
				
				if (result != NULL) return result;
			}
			
			if (value->type()->isRef() && derefType->supportsImplicitCopy()) {
				// Try to copy.
				auto copyValue = derefType->isObject() ?
					CallProperty(derefValue(value), "implicitCopy", std::vector<SEM::Value*>()) :
					derefAll(value) ;
				
				// See if this results in
				// a valid var arg value.
				auto result = VarArgCastSearch(copyValue);
				
				if (result != NULL) return result;
			}
			
			return NULL;
		}
		
		SEM::Value* VarArgCast(SEM::Value* value) {
			auto result = VarArgCastSearch(value);
			if (result == NULL) {
				throw TodoException(makeString("Var arg parameter '%s' has invalid type '%s'.",
					value->toString().c_str(), value->type()->toString().c_str()));
			}
			return result;
		}
		
	}
	
}


