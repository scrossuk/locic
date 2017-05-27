#include <locic/SemanticAnalysis/Unifier.hpp>

#include <locic/AST/Predicate.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/ValueDecl.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		Unifier::Unifier() { }
		
		Diag
		UnifyIncompatibleTypesDiag(const AST::Type* const sourceType,
		                           const AST::Type* const destType) {
			return Error("cannot unify incompatible types '%s' and '%s'",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		Diag
		UnifyAutoWithRefDiag(const AST::Type* const refType) {
			return Error("cannot unify auto with reference type '%s'",
			             refType->toDiagString().c_str());
		}
		
		Diag
		UnifyMismatchingTemplateArgDiag(const AST::Value& first,
		                                const AST::Value& second) {
			return Error("cannot unify template arguments '%s' and '%s'",
			             first.toDiagString().c_str(),
			             second.toDiagString().c_str());
		}
		
		Diag
		UnifyMismatchingConstPredicatesDiag(const AST::Predicate& first,
		                                    const AST::Predicate& second) {
			return Error("cannot unify const predicates '%s' and '%s'",
			             first.toString().c_str(),
			             second.toString().c_str());
		}
		
		ResultOrDiag<const AST::Type*>
		Unifier::unifyTypes(const AST::Type* first,
		                    const AST::Type* second) {
			if (first->isAlias() || second->isAlias()) {
				first = first->resolveAliases();
				second = second->resolveAliases();
			}
			assert(!first->isAlias() && !second->isAlias());
			
			if (first == second) { return second; }
			
			auto constPredicateResult = unifyConstPredicates(first->constPredicate(),
			                                                 second->constPredicate());
			if (constPredicateResult.failed()) return constPredicateResult;
			
			auto nonConstUnifyResult = unifyNonConstTypes(first->stripConst(),
			                                              second->stripConst());
			if (nonConstUnifyResult.failed()) return nonConstUnifyResult;
			
			if (nonConstUnifyResult.value() == second->stripConst()) {
				return second;
			} else {
				return nonConstUnifyResult.value()->applyConst(constPredicateResult.value()->copy());
			}
		}
		
		
		ResultOrDiag<const AST::Type*>
		Unifier::unifyNonConstTypes(const AST::Type* const first,
		                            const AST::Type* const second) {
			assert(!first->hasConst() && !second->hasConst());
			
			if (first->isAuto() || second->isAuto()) {
				const auto otherType = first->isAuto() ? second : first;
				
				if (!otherType->isRef()) {
					// Auto can unify with any non-reference
					// type.
					return otherType;
				} else {
					return UnifyAutoWithRefDiag(otherType);
				}
			}
			
			if (first->kind() != second->kind()) {
				return UnifyIncompatibleTypesDiag(first, second);
			}
			
			if (first->isTemplateVar()) {
				assert(second->isTemplateVar());
				if (first->getTemplateVar() != second->getTemplateVar()) {
					return UnifyIncompatibleTypesDiag(first,
					                                  second);
				}
				
				assert(first == second);
				return first;
			}
			
			assert(first->isObject() && second->isObject());
			
			if (first->getObjectType() != second->getObjectType()) {
				return UnifyIncompatibleTypesDiag(first, second);
			}
			
			assert(first->templateArguments().size() == second->templateArguments().size());
			
			auto argumentsOrDiag = unifyTemplateArgs(first->templateArguments(),
			                                         second->templateArguments());
			if (argumentsOrDiag.failed()) { return argumentsOrDiag; }
			
			return AST::Type::Object(first->getObjectType(),
			                         std::move(argumentsOrDiag.value()));
		}
		
		ResultOrDiag<AST::ValueArray>
		Unifier::unifyTemplateArgs(const AST::ValueArray& first,
		                           const AST::ValueArray& second) {
			assert(first.size() == second.size());
			
			AST::ValueArray unifiedArgs;
			unifiedArgs.reserve(first.size());
			
			for (size_t i = 0; i < first.size(); i++) {
				const auto& firstArg = first[i];
				const auto& secondArg = second[i];
				if (firstArg.isTypeRef() && secondArg.isTypeRef()) {
					auto typeOrDiag = unifyTypes(firstArg.typeRefType(),
					                             secondArg.typeRefType());
					if (typeOrDiag.failed()) return typeOrDiag;
					
					unifiedArgs.push_back(AST::Value::TypeRef(typeOrDiag.value(),
					                                          firstArg.type()));
				} else {
					if (firstArg != secondArg) {
						return UnifyMismatchingTemplateArgDiag(firstArg,
						                                       secondArg);
					}
					
					unifiedArgs.push_back(firstArg.copy());
				}
			}
			
			return ResultOrDiag<AST::ValueArray>(std::move(unifiedArgs));
		}
		
		ResultOrDiag<const AST::Predicate*>
		Unifier::unifyConstPredicates(const AST::Predicate& first,
		                              const AST::Predicate& second) {
			if (first != second) {
				return UnifyMismatchingConstPredicatesDiag(first, second);
			}
			
			return &first;
		}
		
	}
	
}
