#include <locic/SemanticAnalysis/CastGenerator.hpp>

#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/SatisfyChecker.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		CastGenerator::CastGenerator(Context& context,
		                             SatisfyChecker& checker,
		                             const AST::Type* const sourceType,
		                             const bool isNoop,
		                             const bool canBind)
		: context_(context),
		checker_(checker),
		type_(sourceType),
		isNoop_(isNoop),
		canBind_(canBind) {
			assert(!(isNoop_ && canBind_));
			assert(type_->canBeUsedAsValue());
		}
		
		const AST::Type*
		CastGenerator::type() const {
			return type_;
		}
		
		void
		CastGenerator::setSourceType(const AST::Type* const sourceType) {
			assert(sourceType->canBeUsedAsValue());
			type_ = sourceType;
		}
		
		OptionalDiag
		CastGenerator::implicitCast(const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue());
			
			// Keep removing references from source type until we
			// reach depth of destination type.
			while (type()->refDepth() > destType->refDepth()) {
				auto diag = implicitCopyRefToValue();
				if (diag.failed()) return diag;
			}
			
			if (type()->refDepth() == destType->refDepth()) {
				if (type()->isRef()) {
					return implicitCastRefToRef(destType);
				} else {
					return implicitCastValueToValue(destType);
				}
			} else {
				return implicitCastValueToRef(destType);
			}
		}
		
		OptionalDiag
		CastGenerator::implicitCastRefToRef(const AST::Type* const destType) {
			assert(type()->isRef() && destType->isRef());
			assert(type()->refDepth() == destType->refDepth());
			
			// Try a polymorphic reference cast.
			if (destType->refTarget()->isInterface()) {
				return implicitCastPolyRefToRef(destType);
			}
			
			auto noopDiag = implicitCastNoop(destType);
			if (noopDiag.success()) {
				return SUCCESS;
			}
			
			// Reference types aren't compatible so we can try to
			// copy, cast and then bind.
			auto diag = implicitCopyRefToValue();
			if (diag.failed()) return diag;
			
			if (implicitCastValueToRef(destType).success()) {
				return SUCCESS;
			}
			
			return noopDiag;
		}
		
		class CannotBindDiag: public Error {
		public:
			CannotBindDiag(const AST::Type* const sourceType,
			               const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("context does not allow binding type '%s' to '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		class CannotBindMoreThanOnceDiag: public Error {
		public:
			CannotBindMoreThanOnceDiag(const AST::Type* const sourceType,
			                           const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("cannot perform multiple binds (from '%s' to '%s') in a single cast",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		OptionalDiag
		CastGenerator::implicitCastValueToRef(const AST::Type* const destType) {
			assert(destType->isRef() && type()->refDepth() < destType->refDepth());
			
			if (!canBind_) {
				return CannotBindDiag(type(), destType);
			}
			
			assert(!isNoop_);
			
			if ((type()->refDepth() + 1) < destType->refDepth()) {
				return CannotBindMoreThanOnceDiag(type(), destType);
			}
			
			if (destType->refTarget()->isInterface()) {
				setSourceType(TypeBuilder(context_).getRefType(type())); //castChain_.addBind();
				return implicitCastPolyRefToRef(destType);
			}
			
			// Try to cast the source type to the destination type without
			// the const tag; if this is successful we can then bind.
			auto diag = implicitCastValueToValue(destType->refTarget()->stripConst());
			if (diag.failed()) return diag;
			
			setSourceType(TypeBuilder(context_).getRefType(type())); //castChain_.addBind();
			return implicitCastNoop(destType);
		}
		
		class PolyCastFailedDiag: public Error {
		public:
			PolyCastFailedDiag(const AST::Type* const sourceType,
			                   const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("polymorphic cast failed from type '%s' to interface '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		class CannotPolyCastInNoopContextDiag: public Error {
		public:
			CannotPolyCastInNoopContextDiag(const AST::Type* const sourceType,
			                                const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("cannot perform polymorphic cast from type '%s' to interface '%s' in noop cast context",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		OptionalDiag
		CastGenerator::implicitCastPolyRefToRef(const AST::Type* const destType) {
			assert(type()->isRef() && destType->isRef());
			assert(type()->refDepth() == destType->refDepth());
			assert(destType->refTarget()->isInterface());
			
			if (type()->refTarget()->isInterface()) {
				// Interface& -> Interface& should be noop.
				return implicitCastNoop(destType);
			}
			
			auto result = checker_.satisfies(type()->refTarget(),
			                                 destType->refTarget());
			if (result.failed()) {
				// TODO: chain satisfy failure diagnostic.
				return PolyCastFailedDiag(type(), destType);
			}
			
			if (isNoop_) {
				return CannotPolyCastInNoopContextDiag(type(), destType);
			}
			
			setSourceType(destType); //castChain_.addPolyCast(destType);
			return SUCCESS;
		}
		
		class CannotVariantCastInNoopContextDiag: public Error {
		public:
			CannotVariantCastInNoopContextDiag(const AST::Type* const sourceType,
			                                   const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("cannot cast from type '%s' to parent variant '%s' in noop cast context",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		class CannotUserCastInNoopContextDiag: public Error {
		public:
			CannotUserCastInNoopContextDiag(const AST::Type* const sourceType,
			                                const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("cannot perform user-specified cast from type '%s' to type '%s' in noop cast context",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		OptionalDiag
		CastGenerator::implicitCastValueToValue(const AST::Type* const destType) {
			assert(!type()->isRef() && !destType->isRef());
			assert(!destType->isInterface());
			
			auto noopDiag = implicitCastNoop(destType);
			if (noopDiag.success()) return noopDiag;
			
			if (destType->isVariant()) {
				if (implicitCastVariant(destType).success()) {
					if (isNoop_) {
						return CannotVariantCastInNoopContextDiag(type(),
						                                          destType);
					}
					return SUCCESS;
				}
			} else {
				if (implicitCastUser(destType).success()) {
					if (isNoop_) {
						return CannotUserCastInNoopContextDiag(type(),
						                                       destType);
					}
					return SUCCESS;
				}
			}
			
			return noopDiag;
		}
		
		class TypeNotInVariantDiag: public Error {
		public:
			TypeNotInVariantDiag(const AST::Type* const sourceType,
			                     const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("type '%s' is not a member of variant '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		OptionalDiag
		CastGenerator::implicitCastVariant(const AST::Type* const destType) {
			assert(destType->isVariant());
			
			for (const auto variantChildType: destType->getObjectType()->variantTypes()) {
				if (type()->getObjectType() == variantChildType->getObjectType()) {
					setSourceType(destType); //castChain_.addVariantCast(destType);
					return SUCCESS;
				}
			}
			
			return TypeNotInVariantDiag(type(), destType);
		}
		
		class CannotUserCastDiag: public Error {
		public:
			CannotUserCastDiag(const AST::Type* const sourceType,
			                   const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("no user-specified cast exists from '%s' to '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		OptionalDiag
		CastGenerator::implicitCastUser(const AST::Type* const destType) {
			assert(!destType->isInterface());
			
			TypeCapabilities capabilities(context_);
			if (!capabilities.supportsImplicitCast(type(), destType)) {
				return CannotUserCastDiag(type(), destType);
			}
			
			setSourceType(destType); //castChain_.addImplicitCast(destType);
			return SUCCESS;
		}
		
		OptionalDiag
		CastGenerator::implicitCastNoop(const AST::Type* const destType) {
			assert(!type()->isAuto());
			assert(type()->canBeUsedAsValue() && destType->canBeUsedAsValue());
			
			// Special case references and pointers.
			// TODO: Generalise this by looking for implicit cast
			//       methods tagged 'noop'.
			if (type()->isRef() && destType->isRef()) {
				// Prevent polymorphic cast since it is NOT a noop.
				if (!type()->refTarget()->isInterface() &&
				    destType->refTarget()->isInterface()) {
					return CannotPolyCastInNoopContextDiag(type(),
					                                       destType);
				}
				
				auto diag = checker_.satisfies(type()->refTarget(),
				                               destType->refTarget());
				if (diag.failed()) { return diag; }
			} else if (type()->isBuiltInPointer() && destType->isBuiltInPointer()) {
				auto diag = checker_.satisfies(type()->pointeeType(),
				                               destType->pointeeType());
				if (diag.failed()) { return diag; }
			} else {
				// TODO: Support user-specified noop casts.
				auto diag = checker_.satisfies(type(), destType);
				if (diag.failed()) { return diag; }
			}
			
			setSourceType(destType); //castChain_.addNoopCast(destType);
			return SUCCESS;
		}
		
		class CannotCopyInNoopContext: public Error {
		public:
			CannotCopyInNoopContext(const AST::Type* const type)
			: type_(type) { }

			std::string toString() const {
				return makeString("cannot copy type '%s' in noop cast context",
				                  type_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* type_;
			
		};
		
		class CannotCopyDiag: public Error {
		public:
			CannotCopyDiag(const AST::Type* const type)
			: type_(type) { }

			std::string toString() const {
				return makeString("type '%s' is not implicitly copyable",
				                  type_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* type_;
			
		};
		
		OptionalDiag
		CastGenerator::implicitCopyRefToValue() {
			assert(type()->isRef());
			
			// We assume that copying gives the reference target without const.
			const auto copyType = type()->refTarget()->stripConst();
			
			TypeCapabilities capabilities(context_);
			if (!capabilities.supportsImplicitCopy(copyType)) {
				return CannotCopyDiag(copyType);
			}
			
			if (isNoop_) {
				return CannotCopyInNoopContext(copyType);
			}
			
			setSourceType(copyType); //castChain_.addImplicitCopy(type()->refTarget()->stripConst());
			return SUCCESS;
		}
		
	}
	
}
