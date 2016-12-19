#include <cassert>

#include <locic/AST/Type.hpp>
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
		
		TypeCapabilities::TypeCapabilities(Context& context)
		: context_(context) { }
		
		const AST::Type*
		TypeCapabilities::getCapabilityType(const String capability, AST::TypeArray templateArgs) {
			for (auto& arg: templateArgs) {
				arg = arg->resolveAliases();
			}
			
			return getBuiltInType(context_, capability,
			                      std::move(templateArgs))->resolveAliases();
		}
		
		bool
		TypeCapabilities::checkCapabilityWithType(const AST::Type* const rawType,
		                                          const String capability,
		                                          const AST::Type* requireType) {
			const auto type = rawType->resolveAliases();
			if (!type->isObject() && !type->isTemplateVar()) {
				return false;
			}
			
			const Optional<bool> previousResult = context_.getCapability(type, capability);
			if (previousResult) {
				return *previousResult;
			}
			
			const auto sourceMethodSet = getTypeMethodSet(context_, type);
			const auto requireMethodSet = getTypeMethodSet(context_, requireType);
			
			const bool result = methodSetSatisfiesRequirement(context_, sourceMethodSet,
			                                                  requireMethodSet);
			context_.setCapability(type, capability, result);
			return result;
		}
		
		bool
		TypeCapabilities::checkCapability(const AST::Type* const rawType, const String capability,
		                                  AST::TypeArray templateArgs) {
			const auto requireType = getCapabilityType(capability, std::move(templateArgs));
			return checkCapabilityWithType(rawType, capability, requireType);
		}
		
		bool
		TypeCapabilities::hasCallMethod(const AST::Type* type) {
			const auto methodSet = getTypeMethodSet(context_, type);
			const auto methodIterator = methodSet->find(context_.getCString("call"));
			return methodIterator != methodSet->end();
		}
		
		bool
		TypeCapabilities::supportsImplicitCast(const AST::Type* type) {
			switch (type->kind()) {
				case AST::Type::TEMPLATEVAR:
					return false;
					
				case AST::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					const auto function = typeInstance->findFunction(context_.getCString("implicitcast"));
					if (function == nullptr) return false;
					if (function->type().attributes().isVarArg()) return false;
					if (!function->isMethod()) return false;
					if (function->isStaticMethod()) return false;
					
					if (!evaluatePredicate(context_, function->constPredicate(), type->generateTemplateVarMap())) return false;
					if (!function->type().parameterTypes().empty()) return false;
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
		
		bool
		TypeCapabilities::supportsImplicitCopy(const AST::Type* const type) {
			return supportsMove(type->resolveAliases()->withoutTags()) &&
				checkCapability(type, context_.getCString("implicit_copyable_t"),
				                { type->resolveAliases()->withoutTags() });
		}
		
		bool
		TypeCapabilities::supportsNoExceptImplicitCopy(const AST::Type* const type) {
			return supportsMove(type->resolveAliases()->withoutTags()) &&
				checkCapability(type, context_.getCString("noexcept_implicit_copyable_t"),
				                { type->resolveAliases()->withoutTags() });
		}
		
		bool
		TypeCapabilities::supportsExplicitCopy(const AST::Type* const type) {
			return supportsMove(type->resolveAliases()->withoutTags()) &&
				checkCapability(type, context_.getCString("copyable_t"),
				                { type->resolveAliases()->withoutTags() });
		}
		
		bool
		TypeCapabilities::supportsNoExceptExplicitCopy(const AST::Type* const type) {
			return supportsMove(type->resolveAliases()->withoutTags()) &&
				checkCapability(type, context_.getCString("noexcept_copyable_t"),
				                { type->resolveAliases()->withoutTags() });
		}
		
		bool
		TypeCapabilities::supportsCompare(const AST::Type* const type) {
			return checkCapability(type, context_.getCString("comparable_t"),
			                       { type->resolveAliases()->withoutTags() });
		}
		
		bool
		TypeCapabilities::supportsNoExceptCompare(const AST::Type* const type) {
			return checkCapability(type, context_.getCString("noexcept_comparable_t"),
			                       { type->resolveAliases()->withoutTags() });
		}
		
		bool
		TypeCapabilities::supportsMove(const AST::Type* const type) {
			return checkCapabilityWithType(type, context_.getCString("movable_t"),
			                               context_.typeBuilder().getMovableInterfaceType());
		}
		
		bool
		TypeCapabilities::supportsCall(const AST::Type* const type) {
			return checkCapability(type, context_.getCString("callable"), {});
		}
		
	}
	
}


