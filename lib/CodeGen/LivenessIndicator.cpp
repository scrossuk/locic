#include <locic/CodeGen/LivenessIndicator.hpp>

namespace locic {
	
	namespace CodeGen {
		
		LivenessIndicator LivenessIndicator::None() {
			LivenessIndicator indicator(NONE);
			return indicator;
		}
		
		LivenessIndicator LivenessIndicator::MemberInvalidState(const AST::Var& memberVar) {
			LivenessIndicator indicator(MEMBER_INVALID_STATE);
			indicator.data_.memberVar = &memberVar;
			return indicator;
		}
		
		LivenessIndicator LivenessIndicator::CustomMethods() {
			LivenessIndicator indicator(CUSTOM_METHODS);
			return indicator;
		}
		
		LivenessIndicator LivenessIndicator::SuffixByte() {
			LivenessIndicator indicator(SUFFIX_BYTE);
			return indicator;
		}
		
		LivenessIndicator LivenessIndicator::GapByte(const size_t offset) {
			LivenessIndicator indicator(GAP_BYTE);
			indicator.data_.offset = offset;
			return indicator;
		}
		
		LivenessIndicator::LivenessIndicator(const Kind argKind)
		: kind_(argKind) { }
		
		LivenessIndicator::Kind LivenessIndicator::kind() const {
			return kind_;
		}
		
		bool LivenessIndicator::isNone() const {
			return kind() == NONE;
		}
		
		bool LivenessIndicator::isMemberInvalidState() const {
			return kind() == MEMBER_INVALID_STATE;
		}
		
		const AST::Var& LivenessIndicator::memberVar() const {
			assert(isMemberInvalidState());
			return *(data_.memberVar);
		}
		
		bool LivenessIndicator::isCustomMethods() const {
			return kind() == CUSTOM_METHODS;
		}
		
		bool LivenessIndicator::isSuffixByte() const {
			return kind() == SUFFIX_BYTE;
		}
		
		bool LivenessIndicator::isGapByte() const {
			return kind() == GAP_BYTE;
		}
		
		size_t LivenessIndicator::gapByteOffset() const {
			assert(isGapByte());
			return data_.offset;
		}
		
	}
	
}
