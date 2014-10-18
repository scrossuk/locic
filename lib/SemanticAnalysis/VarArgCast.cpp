#include <set>

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		bool isValidVarArgType(Context& context, const SEM::Type* type) {
			if (!type->isObject()) return false;
			if (!type->getObjectType()->isPrimitive()) return false;
			if (type->isLval() || type->isRef()) return false;
			
			const auto name = type->getObjectType()->name().first();
			
			const auto& validTypes = context.validVarArgTypes();
			return validTypes.find(name) != validTypes.end();
		}
		
		static inline SEM::Value* VarArgCastSearch(Context& context, SEM::Value* value, const Debug::SourceLocation& location) {
			if (isValidVarArgType(context, value->type()->resolveAliases())) {
				// Already a valid var arg type.
				return value;
			}
			
			auto derefType = getDerefType(value->type()->resolveAliases());
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
					CallValue(context, GetMethod(context, value, "implicitcopy", location), {}, location) :
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


