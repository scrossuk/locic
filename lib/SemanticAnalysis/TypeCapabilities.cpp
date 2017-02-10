#include <cassert>

#include <locic/AST/MethodSet.hpp>
#include <locic/AST/Type.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/SatisfyChecker.hpp>
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
			
			auto result = SatisfyChecker(context_).satisfies(type, requireType);
			context_.setCapability(type, capability, result.success());
			return result.success();
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
					
					if (evaluatePredicate(context_, function->constPredicate(), type->generateTemplateVarMap()).failed()) return false;
					if (!function->type().parameterTypes().empty()) return false;
					if (function->templateVariables().size() != 1) return false;
					
					const auto returnType = function->type().returnType()->substitute(type->generateTemplateVarMap(),
					                                                                  /*selfconst=*/AST::Predicate::SelfConst());
					
					if (!returnType->isTemplateVar()) return false;
					if (returnType->getTemplateVar() != function->templateVariables().front()) return false;
					
					return true;
				}
					
				default:
					locic_unreachable("Unknown type kind.");
			}
		}
		
		bool
		TypeCapabilities::supportsImplicitCast(const AST::Type* const type,
		                                       const AST::Type* const toType) {
			if (!supportsImplicitCast(type)) return false;
			
			const auto& castFunction = type->getObjectType()->getFunction(context_.getCString("implicitcast"));
			
			const auto& requiresPredicate = castFunction.requiresPredicate();
			
			auto combinedTemplateVarMap = type->generateTemplateVarMap();
			const auto& castTemplateVar = castFunction.templateVariables().front();
			combinedTemplateVarMap.insert(std::make_pair(castTemplateVar, AST::Value::TypeRef(toType, castTemplateVar->type())));
			
			return evaluatePredicate(context_, requiresPredicate, combinedTemplateVarMap).success();
		}
		
		bool
		TypeCapabilities::supportsImplicitCopy(const AST::Type* const type) {
			return isSized(type) &&
				checkCapability(type, context_.getCString("implicit_copyable_t"),
				                { type });
		}
		
		bool
		TypeCapabilities::supportsNoExceptImplicitCopy(const AST::Type* const type) {
			return isSized(type) &&
				checkCapability(type, context_.getCString("noexcept_implicit_copyable_t"),
				                { type });
		}
		
		bool
		TypeCapabilities::supportsExplicitCopy(const AST::Type* const type) {
			return isSized(type) &&
				checkCapability(type, context_.getCString("copyable_t"),
				                { type });
		}
		
		bool
		TypeCapabilities::supportsNoExceptExplicitCopy(const AST::Type* const type) {
			return isSized(type) &&
				checkCapability(type, context_.getCString("noexcept_copyable_t"),
				                { type });
		}
		
		bool
		TypeCapabilities::supportsCompare(const AST::Type* const type) {
			return checkCapability(type, context_.getCString("comparable_t"),
			                       { type });
		}
		
		bool
		TypeCapabilities::supportsNoExceptCompare(const AST::Type* const type) {
			return checkCapability(type, context_.getCString("noexcept_comparable_t"),
			                       { type });
		}
		
		bool
		TypeCapabilities::supportsMove(const AST::Type* const type) {
			return isSized(type) &&
				checkCapability(type, context_.getCString("movable_t"),
				                { type });
		}
		
		bool
		TypeCapabilities::supportsCall(const AST::Type* const type) {
			return checkCapability(type, context_.getCString("callable"), {});
		}
		
		bool
		TypeCapabilities::isSized(const AST::Type* const type) {
			return checkCapability(type, context_.getCString("sized_type_t"), {});
		}
		
	}
	
}


