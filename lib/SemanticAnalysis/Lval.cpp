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
			const auto lvalTypeInstance = getBuiltInType(context.scopeStack(), "value_lval")->getObjectType();
			const auto lvalType = SEM::Type::Object(lvalTypeInstance, { valueType })->createLvalType(valueType);
			return isLvalConst ? lvalType->createConstType() : lvalType;
		}
		
		const SEM::Type* makeMemberLvalType(Context& context, bool isLvalConst, const SEM::Type* valueType) {
			const auto lvalTypeInstance = getBuiltInType(context.scopeStack(), "member_lval")->getObjectType();
			const auto lvalType = SEM::Type::Object(lvalTypeInstance, { valueType })->createLvalType(valueType);
			return isLvalConst ? lvalType->createConstType() : lvalType;
		}
		
		const SEM::Type* makeLvalType(Context& context, bool isMember, bool isLvalConst, const SEM::Type* valueType) {
			if (getDerefType(valueType)->isLval()) return valueType;
			
			if (isMember) {
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
		
		bool supportsDissolve(const SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::AUTO:
				case SEM::Type::ALIAS:
					// Invalid here.
					std::terminate();
				
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
				case SEM::Type::STATICINTERFACEMETHOD:
					return false;
					
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find("dissolve");
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto function = methodIterator->second;
					if (function->type()->isFunctionVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					if (!function->parameters().empty()) return false;
					
					return true;
				}
				
				case SEM::Type::TEMPLATEVAR:
					return supportsDissolve(type->getTemplateVar()->specTypeInstance()->selfType());
			}
			
			std::terminate();
		}
		
		bool canDissolveValue(SEM::Value* value) {
			const auto type = getSingleDerefType(value->type());
			return type->isLval() && type->isObjectOrTemplateVar() && 
supportsDissolve(type);
		}
		
		SEM::Value* dissolveLval(Context& context, SEM::Value* value, const Debug::SourceLocation& location) {
			assert (canDissolveValue(value));
			return CallValue(context, GetSpecialMethod(context, value, "dissolve", location), {}, location);
		}
		
		SEM::Value* tryDissolveValue(Context& context, SEM::Value* value, const Debug::SourceLocation& location) {
			return canDissolveValue(value) ? dissolveLval(context, value, location) : value;
		}
		
	}
	
}


