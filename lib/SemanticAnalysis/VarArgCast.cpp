#include <locic/Debug.hpp>
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
			if (name == "longlong_t" || name == "ulonglong_t") return true;
			
			if (name == "int8_t" || name == "uint8_t") return true;
			if (name == "int16_t" || name == "uint16_t") return true;
			if (name == "int32_t" || name == "uint32_t") return true;
			if (name == "int64_t" || name == "uint64_t") return true;
			
			if (name == "float_t") return true;
			if (name == "double_t") return true;
			if (name == "longdouble_t") return true;
			if (name == "__ptr") return true;
			
			return false;
		}
		
		static inline SEM::Value* VarArgCastSearch(Context& context, SEM::Value* value, const Debug::SourceLocation& location) {
			if (isValidVarArgType(value->type())) {
				// Already a valid var arg type.
				return value;
			}
			
			auto derefType = getDerefType(value->type());
			assert(!derefType->isRef());
			
			if (derefType->isLval() && canDissolveValue(derefValue(value))) {
				// Dissolve lval.
				auto dissolvedValue = dissolveLval(context, derefValue(value), location);
				
				// See if this results in
				// a valid var arg value.
				auto result = VarArgCastSearch(context, dissolvedValue, location);
				
				if (result != nullptr) return result;
			}
			
			if (value->type()->isRef() && supportsImplicitCopy(derefType)) {
				// Try to copy.
				auto copyValue = derefType->isObject() ?
					CallValue(context, GetMethod(context, value, "implicitCopy", location), {}, location) :
					derefAll(value);
				
				// See if this results in
				// a valid var arg value.
				auto result = VarArgCastSearch(context, copyValue, location);
				
				if (result != nullptr) return result;
			}
			
			return nullptr;
		}
		
		SEM::Value* VarArgCast(Context& context, SEM::Value* value, const Debug::SourceLocation& location) {
			auto result = VarArgCastSearch(context, value, location);
			if (result == nullptr) {
				throw ErrorException(makeString("Var arg parameter '%s' has invalid type '%s' at position %s.",
					value->toString().c_str(), value->type()->toString().c_str(),
					location.toString().c_str()));
			}
			return result;
		}
		
	}
	
}


