#include <stdexcept>

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		const SEM::Type* makeValueLvalType(Context& context, bool isLvalConst, const SEM::Type* valueType) {
			const auto lvalType = getBuiltInType(context.scopeStack(), "value_lval", { valueType })->createLvalType(valueType);
			return isLvalConst ? lvalType->createConstType() : lvalType;
		}
		
		const SEM::Type* makeMemberLvalType(Context& context, bool isLvalConst, const SEM::Type* valueType) {
			const auto lvalType = getBuiltInType(context.scopeStack(), "member_lval", { valueType })->createLvalType(valueType);
			return isLvalConst ? lvalType->createConstType() : lvalType;
		}
		
		const SEM::Type* makeFinalLvalType(Context& context, bool isLvalConst, const SEM::Type* valueType) {
			const auto lvalType = getBuiltInType(context.scopeStack(), "final_lval", { valueType })->createLvalType(valueType);
			return isLvalConst ? lvalType->createConstType() : lvalType;
		}
		
		const SEM::Type* makeLvalType(Context& context, bool isMember, bool isFinal, const SEM::Type* valueType) {
			if (getDerefType(valueType)->isLval()) return valueType;
			
			const bool isLvalConst = false;
			
			if (isFinal) {
				return makeFinalLvalType(context, isLvalConst, valueType);
			} else if (isMember) {
				return makeMemberLvalType(context, isLvalConst, valueType);
			} else {
				return makeValueLvalType(context, isLvalConst, valueType);
			}
		}
		
		size_t getLvalCount(const SEM::Type* type) {
			type = getDerefType(type);
			
			size_t count = 0;
			while (type->isLval()) {
				count++;
				type = getDerefType(type->lvalTarget());
			}
			return count;
		}
		
		bool canDissolveType(Context& context, const SEM::Type* const rawType) {
			const auto type = getSingleDerefType(rawType);
			return type->isLval() && type->isObjectOrTemplateVar() && supportsDissolve(context, type);
		}
		
		bool canDissolveValue(Context& context, const SEM::Value& value) {
			return canDissolveType(context, value.type());
		}
		
		SEM::Value dissolveLval(Context& context, SEM::Value value, const Debug::SourceLocation& location) {
			assert (canDissolveValue(context, value));
			return CallValue(context, GetSpecialMethod(context, std::move(value), "dissolve", location), {}, location);
		}
		
		SEM::Value tryDissolveValue(Context& context, SEM::Value value, const Debug::SourceLocation& location) {
			if (canDissolveValue(context, value)) {
				return dissolveLval(context, std::move(value), location);
			} else {
				return value;
			}
		}
		
	}
	
}


