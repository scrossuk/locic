#include <set>

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>
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
			
			if (value.type()->isRef() && TypeCapabilities(context).supportsImplicitCopy(derefType)) {
				// Try to copy.
				auto copyValue = CallValue(context, GetMethod(context, std::move(value),
				                                              context.getCString("implicitcopy"), location),
				                           {}, location);
				
				// See if this results in
				// a valid var arg value.
				auto result = VarArgCastSearch(context, std::move(copyValue), location);
				
				if (result) {
					return result;
				}
			}
			
			return Optional<SEM::Value>();
		}
		
		class VarArgInvalidTypeDiag: public Error {
		public:
			VarArgInvalidTypeDiag(const SEM::Type* const type)
			: typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot pass value of type '%s' to vararg function",
				                  typeString_.c_str());
			}
			
		private:
			std::string typeString_;
			
		};
		
		SEM::Value VarArgCast(Context& context, SEM::Value value, const Debug::SourceLocation& location) {
			const std::string valueString = value.toString();
			const auto valueType = value.type();
			auto result = VarArgCastSearch(context, std::move(value), location);
			if (!result) {
				context.issueDiag(VarArgInvalidTypeDiag(valueType),
				                  location);
				return SEM::Value::CastDummy(valueType);
			}
			return std::move(*result);
		}
		
	}
	
}


