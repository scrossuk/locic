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
		
		bool isValidVarArgType(Context& context, const SEM::Type* const type) {
			if (!type->isObject()) return false;
			if (!type->getObjectType()->isPrimitive()) return false;
			if (type->isLval() || type->isRef()) return false;
			
			const auto& name = type->getObjectType()->name().first();
			const auto& validTypes = context.validVarArgTypes();
			return validTypes.find(name) != validTypes.end();
		}
		
		Optional<SEM::Value> VarArgCastSearch(Context& context, SEM::Value rawValue, const Debug::SourceLocation& location) {
			auto value = derefValue(std::move(rawValue));
			
			if (isValidVarArgType(context, value.type()->resolveAliases())) {
				// Already a valid var arg type.
				return make_optional(std::move(value));
			}
			
			const auto derefType = getDerefType(value.type()->resolveAliases());
			assert(!derefType->isRef());
			
			if (derefType->isLval() && canDissolveValue(context, value)) {
				// Dissolve lval.
				// TODO: remove this value copy!
				auto dissolvedValue = dissolveLval(context, value.copy(), location);
				
				// See if this results in
				// a valid var arg value.
				auto result = VarArgCastSearch(context, std::move(dissolvedValue), location);
				
				if (result) {
					return result;
				}
			}
			
			if (value.type()->isRef() && supportsImplicitCopy(context, derefType)) {
				// Try to copy.
				auto copyValue = CallValue(context, GetMethod(context, std::move(value), context.getCString("implicitcopy"), location), {}, location);
				
				// See if this results in
				// a valid var arg value.
				auto result = VarArgCastSearch(context, std::move(copyValue), location);
				
				if (result) {
					return result;
				}
			}
			
			return Optional<SEM::Value>();
		}
		
		SEM::Value VarArgCast(Context& context, SEM::Value value, const Debug::SourceLocation& location) {
			const std::string valueString = value.toString();
			const auto valueType = value.type();
			auto result = VarArgCastSearch(context, std::move(value), location);
			if (!result) {
				throw ErrorException(makeString("Var arg parameter '%s' has invalid type '%s' at position %s.",
					valueString.c_str(),
					valueType->toString().c_str(),
					location.toString().c_str()));
			}
			return std::move(*result);
		}
		
	}
	
}


