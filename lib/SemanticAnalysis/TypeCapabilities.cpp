#include <cassert>

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		bool checkCapability(Context& context, const SEM::Type* const rawType, const String& capability, SEM::TypeArray templateArgs) {
			const auto type = rawType->resolveAliases();
			if (!type->isObject() && !type->isTemplateVar()) {
				return false;
			}
			
			const Optional<bool> previousResult = context.getCapability(type, capability);
			if (previousResult) {
				return *previousResult;
			}
			
			for (auto& arg: templateArgs) {
				arg = arg->resolveAliases();
			}
			
			const auto requireType = getBuiltInType(context, capability, std::move(templateArgs))->resolveAliases();
			
			const auto sourceMethodSet = getTypeMethodSet(context, type);
			const auto requireMethodSet = getTypeMethodSet(context, requireType);
			
			const bool result = methodSetSatisfiesRequirement(context, sourceMethodSet, requireMethodSet);
			context.setCapability(type, capability, result);
			return result;
		}
		
		bool supportsImplicitCast(Context& context, const SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
				case SEM::Type::INTERFACEMETHOD:
				case SEM::Type::TEMPLATEVAR:
					return false;
					
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto methodIterator = typeInstance->functions().find(context.getCString("implicitcast"));
					if (methodIterator == typeInstance->functions().end()) return false;
					
					const auto& function = methodIterator->second;
					if (function->type().attributes().isVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					
					// Conservatively assume method is not const if result is undetermined.
					const bool isConstMethodDefault = false;
					
					if (!evaluatePredicateWithDefault(context, function->constPredicate(), type->generateTemplateVarMap(), isConstMethodDefault)) return false;
					if (!function->parameters().empty()) return false;
					if (function->templateVariables().size() != 1) return false;
					
					const auto returnType = function->type().returnType()->substitute(type->generateTemplateVarMap());
					
					if (!returnType->isTemplateVar()) return false;
					if (returnType->getTemplateVar() != function->templateVariables().front()) return false;
					
					return true;
				}
					
				default:
					throw std::runtime_error("Unknown SEM type kind.");
			}
		}
		
		bool supportsImplicitCopy(Context& context, const SEM::Type* const type) {
			return supportsMove(context, type->resolveAliases()->withoutTags()) &&
				checkCapability(context, type, context.getCString("implicit_copyable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptImplicitCopy(Context& context, const SEM::Type* const type) {
			return supportsMove(context, type->resolveAliases()->withoutTags()) &&
				checkCapability(context, type, context.getCString("noexcept_implicit_copyable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsExplicitCopy(Context& context, const SEM::Type* const type) {
			return supportsMove(context, type->resolveAliases()->withoutTags()) &&
				checkCapability(context, type, context.getCString("copyable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptExplicitCopy(Context& context, const SEM::Type* const type) {
			return supportsMove(context, type->resolveAliases()->withoutTags()) &&
				checkCapability(context, type, context.getCString("noexcept_copyable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsCompare(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("comparable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsNoExceptCompare(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("noexcept_comparable"), { type->resolveAliases()->withoutTags() });
		}
		
		bool supportsMove(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("movable"), {});
		}
		
		bool supportsDissolve(Context& context, const SEM::Type* const type) {
			assert(type->isLval());
			return checkCapability(context, type, context.getCString("dissolvable"), { type->lvalTarget() }) ||
				checkCapability(context, type, context.getCString("const_dissolvable"), { type->lvalTarget() });
		}
		
		bool supportsCall(Context& context, const SEM::Type* const type) {
			return checkCapability(context, type, context.getCString("callable"), {});
		}
		
	}
	
}


