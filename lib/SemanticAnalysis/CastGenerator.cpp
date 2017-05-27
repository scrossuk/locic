#include <locic/SemanticAnalysis/CastGenerator.hpp>

#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/CastOperation.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/SatisfyChecker.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		CastGenerator::CastGenerator(Context& context,
		                             SatisfyChecker& checker)
		: context_(context),
		checker_(checker) { }
		
		ResultOrDiag<CastOperation>
		CastGenerator::implicitCast(const AST::Type* const sourceType,
		                            const AST::Type* const destType,
		                            const bool canBind) {
			assert(sourceType->canBeUsedAsValue());
			assert(destType->canBeUsedAsValue());
			
			CastOperation cast(context_, sourceType,
			                   /*isNoop=*/false, canBind);
			
			auto result = implicitCastAnyToAny(cast, destType);
			if (result.failed()) return result.extractDiag();
			
			return cast;
		}
		
		ResultOrDiag<CastOperation>
		CastGenerator::implicitCastNoop(const AST::Type* const sourceType,
		                                const AST::Type* const destType) {
			assert(sourceType->canBeUsedAsValue());
			assert(destType->canBeUsedAsValue());
			
			CastOperation cast(context_, sourceType,
			                   /*isNoop=*/true, /*canBind=*/false);
			
			auto result = implicitCastAnyToAny(cast, destType);
			if (result.failed()) return result.extractDiag();
			
			return cast;
		}
		
		OptionalDiag
		CastGenerator::implicitCastAnyToAny(CastOperation& cast,
		                                    const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue());
			
			// Keep removing references from source type until we
			// reach depth of destination type.
			while (cast.type()->refDepth() > destType->refDepth()) {
				auto diag = implicitCopyRefToValue(cast);
				if (diag.failed()) return diag;
			}
			
			if (cast.type()->refDepth() == destType->refDepth()) {
				if (cast.type()->isRef()) {
					return implicitCastRefToRef(cast, destType);
				} else {
					return implicitCastValueToValue(cast, destType);
				}
			} else {
				return implicitCastValueToRef(cast, destType);
			}
		}
		
		OptionalDiag
		CastGenerator::implicitCastRefToRef(CastOperation& cast,
		                                    const AST::Type* const destType) {
			assert(cast.type()->isRef() && destType->isRef());
			assert(cast.type()->refDepth() == destType->refDepth());
			
			// Try a polymorphic reference cast.
			if (destType->refTarget()->isInterface()) {
				return implicitCastPolyRefToRef(cast, destType);
			}
			
			auto noopDiag = implicitCastNoopOnly(cast, destType);
			if (noopDiag.success()) { return SUCCESS; }
			
			// Reference types aren't compatible so we can try to
			// copy, cast and then bind.
			auto diag = implicitCopyRefToValue(cast);
			if (diag.failed()) return diag;
			
			if (implicitCastValueToRef(cast, destType).success()) {
				return SUCCESS;
			}
			
			return noopDiag;
		}
		
		Diag
		CannotBindDiag(const AST::Type* const sourceType,
		               const AST::Type* const destType) {
			return Error("context does not allow binding type '%s' to '%s'",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		Diag
		CannotBindMoreThanOnceDiag(const AST::Type* const sourceType,
		                           const AST::Type* const destType) {
			return Error("cannot perform multiple binds (from '%s' to '%s') in a single cast",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		OptionalDiag
		CastGenerator::implicitCastValueToRef(CastOperation& cast,
		                                      const AST::Type* const destType) {
			assert(destType->isRef());
			assert(cast.type()->refDepth() < destType->refDepth());
			
			if (!cast.canBind()) {
				return CannotBindDiag(cast.type(), destType);
			}
			
			if ((cast.type()->refDepth() + 1) < destType->refDepth()) {
				return CannotBindMoreThanOnceDiag(cast.type(), destType);
			}
			
			if (destType->refTarget()->isInterface()) {
				cast.addBind();
				return implicitCastPolyRefToRef(cast, destType);
			}
			
			// Try to cast the source type to the destination type without
			// the const tag; if this is successful we can then bind.
			auto diag = implicitCastValueToValue(cast,
			                                     destType->refTarget()->stripConst());
			if (diag.failed()) return diag;
			
			cast.addBind();
			return implicitCastNoopOnly(cast, destType);
		}
		
		Diag
		PolyCastFailedDiag(const AST::Type* const sourceType,
		                   const AST::Type* const destType) {
			return Error("polymorphic cast failed from type '%s' to interface '%s'",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		Diag
		CannotPolyCastInNoopContextDiag(const AST::Type* const sourceType,
		                                const AST::Type* const destType) {
			return Error("cannot perform polymorphic cast from type '%s' to interface '%s' in noop cast context",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		OptionalDiag
		CastGenerator::implicitCastPolyRefToRef(CastOperation& cast,
		                                        const AST::Type* const destType) {
			assert(cast.type()->isRef() && destType->isRef());
			assert(cast.type()->refDepth() == destType->refDepth());
			assert(destType->refTarget()->isInterface());
			
			if (cast.type()->refTarget()->isInterface()) {
				// Interface& -> Interface& should be noop.
				return implicitCastNoopOnly(cast, destType);
			}
			
			auto result = checker_.satisfies(cast.type()->refTarget(),
			                                 destType->refTarget());
			if (result.failed()) {
				// TODO: chain satisfy failure diagnostic.
				return PolyCastFailedDiag(cast.type(), destType);
			}
			
			if (cast.isNoop()) {
				return CannotPolyCastInNoopContextDiag(cast.type(),
				                                       destType);
			}
			
			cast.addPolyCast(destType);
			return SUCCESS;
		}
		
		Diag
		CannotVariantCastInNoopContextDiag(const AST::Type* const sourceType,
		                                   const AST::Type* const destType) {
			return Error("cannot cast from type '%s' to parent variant '%s' in noop cast context",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		Diag
		CannotUserCastInNoopContextDiag(const AST::Type* const sourceType,
		                                const AST::Type* const destType) {
			return Error("cannot perform user-specified cast from type '%s' to type '%s' in noop cast context",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		OptionalDiag
		CastGenerator::implicitCastValueToValue(CastOperation& cast,
		                                        const AST::Type* const destType) {
			assert(!cast.type()->isRef() && !destType->isRef());
			assert(destType->canBeUsedAsValue());
			
			auto noopDiag = implicitCastNoopOnly(cast, destType);
			if (noopDiag.success()) return noopDiag;
			
			if (destType->isVariant()) {
				if (cast.type()->getObjectType()->isMemberOfVariant(*(destType->getObjectType()))) {
					if (cast.isNoop()) {
						return CannotVariantCastInNoopContextDiag(cast.type(),
						                                          destType);
					}
					
					cast.addVariantCast(destType);
					return SUCCESS;
				}
			} else {
				TypeCapabilities capabilities(context_);
				if (capabilities.supportsImplicitCast(cast.type(), destType)) {
					if (cast.isNoop()) {
						return CannotUserCastInNoopContextDiag(cast.type(),
						                                       destType);
					}
					
					cast.addUserCast(destType);
					return SUCCESS;
				}
			}
			
			return noopDiag;
		}
		
		OptionalDiag
		CastGenerator::implicitCastNoopOnly(CastOperation& cast,
		                                    const AST::Type* const destType) {
			assert(!cast.type()->isAuto());
			assert(destType->canBeUsedAsValue());
			
			// Special case references and pointers.
			// TODO: Generalise this by looking for implicit cast
			//       methods tagged 'noop'.
			if (cast.type()->isRef() && destType->isRef()) {
				// Prevent polymorphic cast since it is NOT a noop.
				if (!cast.type()->refTarget()->isInterface() &&
				    destType->refTarget()->isInterface()) {
					return CannotPolyCastInNoopContextDiag(cast.type(),
					                                       destType);
				}
				
				auto result = checker_.satisfies(cast.type()->refTarget(),
				                                 destType->refTarget());
				if (result.failed()) { return result; }
				
				// No need to apply const to resolved type.
				assert(!cast.type()->hasConst() && !destType->hasConst());
				
				TypeBuilder typeBuilder(cast.context());
				const auto resolvedType = typeBuilder.getRefType(result.value());
				cast.addNoopCast(resolvedType);
				return SUCCESS;
			} else if (cast.type()->isBuiltInPointer() &&
			           destType->isBuiltInPointer()) {
				auto result = checker_.satisfies(cast.type()->pointeeType(),
				                                 destType->pointeeType());
				if (result.failed()) { return result; }
				
				// No need to apply const to resolved type.
				assert(!cast.type()->hasConst() && !destType->hasConst());
				
				TypeBuilder typeBuilder(cast.context());
				const auto resolvedType = typeBuilder.getPointerType(result.value());
				cast.addNoopCast(resolvedType);
				return SUCCESS;
			} else {
				auto result = checker_.satisfies(cast.type(), destType);
				if (result.failed()) { return result; }
				
				cast.addNoopCast(result.value());
				return SUCCESS;
			}
		}
		
		Diag
		CannotCopyInNoopContext(const AST::Type* const type) {
			return Error("cannot copy type '%s' in noop cast context",
			             type->toDiagString().c_str());
		}
		
		Diag
		CannotCopyDiag(const AST::Type* const type) {
			return Error("type '%s' is not implicitly copyable",
			             type->toDiagString().c_str());
		}
		
		OptionalDiag
		CastGenerator::implicitCopyRefToValue(CastOperation& cast) {
			assert(cast.type()->isRef());
			
			// We assume that copying gives the reference target without const.
			const auto copyType = cast.type()->refTarget()->stripConst();
			
			TypeCapabilities capabilities(context_);
			if (!capabilities.supportsImplicitCopy(copyType)) {
				return CannotCopyDiag(copyType);
			}
			
			if (cast.isNoop()) {
				return CannotCopyInNoopContext(copyType);
			}
			
			cast.addCopy(copyType);
			return SUCCESS;
		}
		
	}
	
}
