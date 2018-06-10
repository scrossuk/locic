#include <locic/SemanticAnalysis/CastSequence.hpp>

#include <locic/AST/Type.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		CastSequence::CastSequence(Context& context,
		                           const AST::Type* const sourceType)
		: context_(context), type_(sourceType) {
			assert(type_ != nullptr);
			assert(type_->canBeUsedAsValue());
		}
		
		const AST::Type* CastSequence::type() const {
			return type_;
		}
		
		void CastSequence::addBind() {
			type_ = TypeBuilder(context_).getRefType(type_);
			assert(type_->canBeUsedAsValue());
		}
		
		void
		CastSequence::addPolyCast(const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue() && destType->isRef());
			type_ = destType;
		}
		
		void
		CastSequence::addNoopCast(const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastSequence::addCopy(const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastSequence::addUserCast(const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastSequence::addVariantCast(const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue() && destType->isVariant());
			type_ = destType;
		}
		
	}
	
}
