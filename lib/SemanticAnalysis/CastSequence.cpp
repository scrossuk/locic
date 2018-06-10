#include <locic/SemanticAnalysis/CastSequence.hpp>

#include <locic/AST/Type.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		CastSequence::CastSequence(Context& context,
		                             const AST::Type* const sourceType,
		                             const bool isNoop,
		                             const bool canBind)
		: context_(context), type_(sourceType), isNoop_(isNoop),
		canBind_(canBind) {
			assert(!(isNoop_ && canBind_));
			assert(type_ != nullptr);
			assert(type_->canBeUsedAsValue());
		}
		
		Context& CastSequence::context() {
			return context_;
		}
		
		const AST::Type* CastSequence::type() const {
			return type_;
		}
		
		bool CastSequence::isNoop() const {
			return isNoop_;
		}
		
		bool CastSequence::canBind() const {
			return canBind_;
		}
		
		void CastSequence::addBind() {
			assert(!isNoop() && canBind());
			type_ = TypeBuilder(context_).getRefType(type_);
			assert(type_->canBeUsedAsValue());
		}
		
		void
		CastSequence::addPolyCast(const AST::Type* const destType) {
			assert(!isNoop());
			assert(destType->canBeUsedAsValue());
			assert(destType->isRef());
			type_ = destType;
		}
		
		void
		CastSequence::addNoopCast(const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastSequence::addCopy(const AST::Type* const destType) {
			assert(!isNoop());
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastSequence::addUserCast(const AST::Type* const destType) {
			assert(!isNoop());
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastSequence::addVariantCast(const AST::Type* const destType) {
			assert(!isNoop());
			assert(destType->canBeUsedAsValue());
			assert(destType->isVariant());
			type_ = destType;
		}
		
	}
	
}
