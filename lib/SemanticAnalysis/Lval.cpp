#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Type* makeValueLvalType(Context& context, bool isLvalConst, SEM::Type* valueType) {
			const auto lvalTypeInstance = getBuiltInType(context, "value_lval");
			const auto lvalType = SEM::Type::Object(lvalTypeInstance, std::vector<SEM::Type*>(1, valueType))->createLvalType(valueType);
			return isLvalConst ? lvalType->createConstType() : lvalType;
		}
		
		SEM::Type* makeMemberLvalType(Context& context, bool isLvalConst, SEM::Type* valueType) {
			const auto lvalTypeInstance = getBuiltInType(context, "member_lval");
			const auto lvalType = SEM::Type::Object(lvalTypeInstance, std::vector<SEM::Type*>(1, valueType))->createLvalType(valueType);
			return isLvalConst ? lvalType->createConstType() : lvalType;
		}
		
		SEM::Type* makeLvalType(Context& context, bool isMember, bool isLvalConst, SEM::Type* valueType) {
			if (getDerefType(valueType)->isLval()) return valueType;
			
			if (isMember) {
				return makeMemberLvalType(context, isLvalConst, valueType);
			} else {
				return makeValueLvalType(context, isLvalConst, valueType);
			}
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
			const auto type = getDerefType(value->type());
			
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
			
			return CallPropertyMethod(derefValue(value), "dissolve", std::vector<SEM::Value*>());
		}
		
		SEM::Value* tryDissolveValue(SEM::Value* value) {
			return canDissolveValue(value) ? dissolveLval(value) : value;
		}
		
	}
	
}


