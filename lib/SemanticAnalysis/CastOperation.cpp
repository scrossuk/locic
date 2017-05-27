#include <locic/SemanticAnalysis/CastOperation.hpp>

#include <locic/AST/Type.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		CastOperation::CastOperation(Context& context,
		                             const AST::Type* const sourceType,
		                             const bool isNoop,
		                             const bool canBind)
		: context_(context), type_(sourceType), isNoop_(isNoop),
		canBind_(canBind) {
			assert(!(isNoop_ && canBind_));
			assert(type_ != nullptr);
			assert(type_->canBeUsedAsValue());
		}
		
		Context& CastOperation::context() {
			return context_;
		}
		
		const AST::Type* CastOperation::type() const {
			return type_;
		}
		
		bool CastOperation::isNoop() const {
			return isNoop_;
		}
		
		bool CastOperation::canBind() const {
			return canBind_;
		}
		
		void CastOperation::addBind() {
			assert(!isNoop() && canBind());
			type_ = TypeBuilder(context_).getRefType(type_);
			assert(type_->canBeUsedAsValue());
		}
		
		void
		CastOperation::addPolyCast(const AST::Type* const destType) {
			assert(!isNoop());
			assert(destType->canBeUsedAsValue());
			assert(destType->isRef());
			type_ = destType;
		}
		
		void
		CastOperation::addNoopCast(const AST::Type* const destType) {
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastOperation::addCopy(const AST::Type* const destType) {
			assert(!isNoop());
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastOperation::addUserCast(const AST::Type* const destType) {
			assert(!isNoop());
			assert(destType->canBeUsedAsValue());
			type_ = destType;
		}
		
		void
		CastOperation::addVariantCast(const AST::Type* const destType) {
			assert(!isNoop());
			assert(destType->canBeUsedAsValue());
			assert(destType->isVariant());
			type_ = destType;
		}
		
	}
	
}
