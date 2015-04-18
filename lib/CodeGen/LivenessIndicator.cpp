#include <locic/CodeGen/LivenessIndicator.hpp>

namespace locic {
	
	namespace SEM {
		
		class Var;
		
	}
	
	namespace CodeGen {
		
		LivenessIndicator LivenessIndicator::None() {
			LivenessIndicator indicator(NONE);
			return indicator;
		}
		
		LivenessIndicator LivenessIndicator::MemberInvalidState(const SEM::Var& memberVar) {
			LivenessIndicator indicator(MEMBER_INVALID_STATE);
			indicator.data_.memberVar = &memberVar;
			return indicator;
		}
		
		LivenessIndicator LivenessIndicator::CustomMethods() {
			LivenessIndicator indicator(CUSTOM_METHODS);
			return indicator;
		}
		
		LivenessIndicator LivenessIndicator::PrefixByte() {
			LivenessIndicator indicator(PREFIX_BYTE);
			return indicator;
		}
		
		LivenessIndicator LivenessIndicator::GapByte(const size_t index) {
			LivenessIndicator indicator(GAP_BYTE);
			indicator.data_.index = index;
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
		
		const SEM::Var& LivenessIndicator::memberVar() const {
			assert(isMemberInvalidState());
			return *(data_.memberVar);
		}
		
		bool LivenessIndicator::isCustomMethods() const {
			return kind() == CUSTOM_METHODS;
		}
		
		bool LivenessIndicator::isPrefixByte() const {
			return kind() == PREFIX_BYTE;
		}
		
		bool LivenessIndicator::isGapByte() const {
			return kind() == GAP_BYTE;
		}
		
		size_t LivenessIndicator::gapByteIndex() const {
			assert(isGapByte());
			return data_.index;
		}
		
	}
	
}
