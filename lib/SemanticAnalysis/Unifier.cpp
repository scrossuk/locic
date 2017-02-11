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
		
		OptionalDiag
		Unifier::unifyTypes(const AST::Type* first,
		                    const AST::Type* second) {
			if (first->isAlias() || second->isAlias()) {
				printf("ALIASES: %s or %s\n", first->toDiagString().c_str(),
				       second->toDiagString().c_str());
				first = first->resolveAliases();
				second = second->resolveAliases();
			}
			assert(!first->isAlias() && !second->isAlias());
			
			if (first == second) { return SUCCESS; }
			
			auto diag = unifyConstPredicates(first->constPredicate(),
			                                 second->constPredicate());
			if (diag.failed()) return diag;
			
			if (first->isAuto() || second->isAuto()) {
				const auto otherType = first->isAuto() ? second : first;
				
				if (!otherType->isRef()) {
					// Auto can unify with any non-reference
					// type.
					return SUCCESS;
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
			}
			
			assert(first->isObject() && second->isObject());
			
			if (first->getObjectType() != second->getObjectType()) {
				return UnifyIncompatibleTypesDiag(first, second);
			}
			
			assert(first->templateArguments().size() == second->templateArguments().size());
			
			return unifyTemplateArgs(first->templateArguments(),
			                         second->templateArguments());
		}
		
		OptionalDiag
		Unifier::unifyTemplateArgs(const AST::ValueArray& first,
		                           const AST::ValueArray& second) {
			assert(first.size() == second.size());
			
			for (size_t i = 0; i < first.size(); i++) {
				const auto& firstArg = first[i];
				const auto& secondArg = second[i];
				if (firstArg.isTypeRef() && secondArg.isTypeRef()) {
					auto diag = unifyTypes(firstArg.typeRefType(),
					                       secondArg.typeRefType());
					if (diag.failed()) return diag;
				} else if (firstArg != secondArg) {
					return UnifyMismatchingTemplateArgDiag(firstArg,
					                                       secondArg);
				}
			}
			
			return SUCCESS;
		}
		
		OptionalDiag
		Unifier::unifyConstPredicates(const AST::Predicate& first,
		                              const AST::Predicate& second) {
			if (first != second) {
				return UnifyMismatchingConstPredicatesDiag(first, second);
			}
			
			return SUCCESS;
		}
		
	}
	
}
