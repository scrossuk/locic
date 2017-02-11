#include <set>

#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/Debug.hpp>

#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		bool isValidVarArgType(const AST::Type* const type) {
			if (!type->isObject()) return false;
			if (!type->getObjectType()->isPrimitive()) return false;
			if (!type->isPrimitive()) return false;
			
			switch (type->primitiveID()) {
				case PrimitiveByte:
				case PrimitiveUByte:
				case PrimitiveShort:
				case PrimitiveUShort:
				case PrimitiveInt:
				case PrimitiveUInt:
				case PrimitiveLong:
				case PrimitiveULong:
				case PrimitiveLongLong:
				case PrimitiveULongLong:
				case PrimitiveInt8:
				case PrimitiveUInt8:
				case PrimitiveInt16:
				case PrimitiveUInt16:
				case PrimitiveInt32:
				case PrimitiveUInt32:
				case PrimitiveInt64:
				case PrimitiveUInt64:
				case PrimitiveFloat:
				case PrimitiveDouble:
				case PrimitiveLongDouble:
				case PrimitivePtr:
				case PrimitiveSize:
				case PrimitiveSSize:
					return true;
				default:
					return false;
			}
		}
		
		Optional<AST::Value> VarArgCastSearch(Context& context, AST::Value rawValue, const Debug::SourceLocation& location) {
			auto value = derefValue(std::move(rawValue));
			
			if (isValidVarArgType(value.type()->resolveAliases())) {
				// Already a valid var arg type.
				return make_optional(std::move(value));
			}
			
			const auto derefType = getDerefType(value.type()->resolveAliases());
			assert(!derefType->isRef());
			
			if (value.type()->isRef() && TypeCapabilities(context).supportsImplicitCopy(derefType)) {
				// Call implicitcopy() method.
				auto copyValue = CallValue(context, GetMethod(context, std::move(value),
				                                              context.getCString("implicitcopy"), location),
				                           {}, location);
				
				// See if this results in a valid var arg value.
				return VarArgCastSearch(context, std::move(copyValue), location);
			}
			
			return Optional<AST::Value>();
		}
		
		class VarArgInvalidTypeDiag: public ErrorDiag {
		public:
			VarArgInvalidTypeDiag(const AST::Type* const type)
			: typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot pass value of type '%s' to vararg function",
				                  typeString_.c_str());
			}
			
		private:
			std::string typeString_;
			
		};
		
		AST::Value VarArgCast(Context& context, AST::Value value, const Debug::SourceLocation& location) {
			const std::string valueString = value.toString();
			const auto valueType = value.type();
			auto result = VarArgCastSearch(context, std::move(value), location);
			if (!result) {
				context.issueDiag(VarArgInvalidTypeDiag(valueType),
				                  location);
				return AST::Value::CastDummy(valueType);
			}
			return std::move(*result);
		}
		
	}
	
}


