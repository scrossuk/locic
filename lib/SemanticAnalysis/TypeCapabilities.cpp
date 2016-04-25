#include <cassert>

#include <locic/Frontend/OptionalDiag.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/MethodSetSatisfies.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		const SEM::Type* getCapabilityType(Context& context, const String capability, SEM::TypeArray templateArgs) {
			for (auto& arg: templateArgs) {
				arg = arg->resolveAliases();
			}
			
			return getBuiltInType(context, capability, std::move(templateArgs))->resolveAliases();
		}
		
		bool checkCapabilityWithType(Context& context, const SEM::Type* const rawType, const String capability, const SEM::Type* requireType) {
			const auto type = rawType->resolveAliases();
			if (!type->isObject() && !type->isTemplateVar()) {
				return false;
			}
			
			const Optional<bool> previousResult = context.getCapability(type, capability);
			if (previousResult) {
				return *previousResult;
			}
			
			const auto sourceMethodSet = getTypeMethodSet(context, type);
			const auto requireMethodSet = getTypeMethodSet(context, requireType);
			
			const bool result = methodSetSatisfiesRequirement(context, sourceMethodSet, requireMethodSet);
			context.setCapability(type, capability, result);
			return result;
		}
		
		bool checkCapability(Context& context, const SEM::Type* const rawType, const String capability, SEM::TypeArray templateArgs) {
			return checkCapabilityWithType(context, rawType, capability, getCapabilityType(context, capability, std::move(templateArgs)));
		}
		
		bool hasCallMethod(Context& context, const SEM::Type* type) {
			const auto methodSet = getTypeMethodSet(context, type);
			const auto methodIterator = methodSet->find(context.getCString("call"));
			return methodIterator != methodSet->end();
		}
		
		bool supportsImplicitCast(Context& context, const SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::TEMPLATEVAR:
					return false;
					
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto function = typeInstance->findFunction(context.getCString("implicitcast"));
					if (function == nullptr) return false;
					if (function->type().attributes().isVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					
					if (!evaluatePredicate(context, function->constPredicate(), type->generateTemplateVarMap())) return false;
					if (!function->parameters().empty()) return false;
					if (function->templateVariables().size() != 1) return false;
					
					const auto returnType = function->type().returnType()->substitute(type->generateTemplateVarMap());
					
					if (!returnType->isTemplateVar()) return false;
					if (returnType->getTemplateVar() != function->templateVariables().front()) return false;
					
					return true;
				}
					
				default:
					locic_unreachable("Unknown SEM type kind.");
			}
		}
		
		bool supportsImplicitCopy(Context& context, const SEM::Type* const type) {
			return supportsMove(context, type->resolveAliases()->withoutTags()) &&
				checkCapability(context, type, context.getCString("implicit_copyable_t"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptImplicitCopy(Context& context, const SEM::Type* const type) {
			return supportsMove(context, type->resolveAliases()->withoutTags()) &&
				checkCapability(context, type, context.getCString("noexcept_implicit_copyable_t"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsExplicitCopy(Context& context, const SEM::Type* const type) {
			return supportsMove(context, type->resolveAliases()->withoutTags()) &&
				checkCapability(context, type, context.getCString("copyable_t"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptExplicitCopy(Context& context, const SEM::Type* const type) {
			return supportsMove(context, type->resolveAliases()->withoutTags()) &&
				checkCapability(context, type, context.getCString("noexcept_copyable_t"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsCompare(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("comparable_t"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptCompare(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("noexcept_comparable_t"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsMove(Context& context, const SEM::Type* const type) {
			return checkCapabilityWithType(context, type, context.getCString("movable_t"), context.typeBuilder().getMovableInterfaceType());
		}
		
		bool supportsDissolve(Context& context, const SEM::Type* const type) {
			assert(type->isLval());
			return checkCapability(context, type, context.getCString("dissolvable_t"), { type->lvalTarget() }) ||
				checkCapability(context, type, context.getCString("const_dissolvable_t"), { type->lvalTarget() });
		}
		
		bool supportsCall(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("callable"), {});
		}
		
	}
	
}


