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
			
			const auto name = type->getObjectType()->name().first();
			
			// TODO: find a better (cleaner) way to do this...
			if (name == "char_t" || name == "uchar_t") return true;
			if (name == "short_t" || name == "ushort_t") return true;
			if (name == "int_t" || name == "uint_t") return true;
			if (name == "long_t" || name == "ulong_t") return true;
			
			if (name == "int8_t" || name == "uint8_t") return true;
			if (name == "int16_t" || name == "uint16_t") return true;
			if (name == "int32_t" || name == "uint32_t") return true;
			if (name == "int64_t" || name == "uint64_t") return true;
			
			if (name == "float_t") return true;
			if (name == "double_t") return true;
			if (name == "longdouble_t") return true;
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
					CallPropertyMethod(derefValue(value), "implicitCopy", std::vector<SEM::Value*>()) :
					derefAll(value);
				
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


