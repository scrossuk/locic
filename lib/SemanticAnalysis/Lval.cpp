#include <stdexcept>

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
		
		bool supportsDissolve(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
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
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
		bool canDissolveValue(SEM::Value* value) {
			const auto type = getDerefType(value->type());
			return type->isLval() && type->isObject() && supportsDissolve(type);
		}
		
		SEM::Value* dissolveLval(SEM::Value* value) {
			const auto type = getDerefType(value->type());
			
			if (!type->isLval()) {
				throw ErrorException(makeString("Type '%s' is not an lval and hence it cannot be dissolved.",
					type->toString().c_str()));
			}
			
			if (!type->isObject()) {
				throw ErrorException(makeString("Type '%s' is not an object type and hence it cannot be dissolved.",
					type->toString().c_str()));
			}
			
			if (!supportsDissolve(type)) {
				throw ErrorException(makeString("Type '%s' does not support 'dissolve'.",
					type->toString().c_str()));
			}
			
			// TODO: fix location.
			return CallValue(GetMethod(value, "dissolve"), {}, Debug::SourceLocation::Null());
		}
		
		SEM::Value* tryDissolveValue(SEM::Value* value) {
			return canDissolveValue(value) ? dissolveLval(value) : value;
		}
		
	}
	
}


