#ifndef LOCIC_SEM_EXITSTATES_HPP
#define LOCIC_SEM_EXITSTATES_HPP

namespace locic {

	namespace SEM {
		
		class ExitStates {
		public:
			static ExitStates None() {
				return ExitStates();
			}
			
			static ExitStates Normal() {
				return ExitStates(StateNormal);
			}
			
			static ExitStates Return() {
				return ExitStates(StateReturn);
			}
			
			static ExitStates Break() {
				return ExitStates(StateBreak);
			}
			
			static ExitStates Continue() {
				return ExitStates(StateContinue);
			}
			
			static ExitStates Throw() {
				return ExitStates(StateThrow);
			}
			
			static ExitStates Rethrow() {
				return ExitStates(StateRethrow);
			}
			
			ExitStates(const ExitStates&) = default;
			ExitStates& operator=(const ExitStates&) = default;
			
			bool hasNormalExit() const {
				return test(StateNormal);
			}
			
			bool hasReturnExit() const {
				return test(StateReturn);
			}
			
			bool hasBreakExit() const {
				return test(StateBreak);
			}
			
			bool hasContinueExit() const {
				return test(StateContinue);
			}
			
			bool hasThrowExit() const {
				return test(StateThrow);
			}
			
			bool hasRethrowExit() const {
				return test(StateRethrow);
			}
			
			ExitStates remove(const ExitStates toRemove) const {
				return *this & ~toRemove;
			}
			
			ExitStates operator~() const {
				ExitStates newStates;
				newStates.states_ = ~states_;
				return newStates;
			}
			
			ExitStates operator&(const ExitStates& other) const {
				ExitStates newStates;
				newStates.states_ = states_ & other.states_;
				return newStates;
			}
			
			ExitStates operator|(const ExitStates& other) const {
				ExitStates newStates;
				newStates.states_ = states_ | other.states_;
				return newStates;
			}
			
			ExitStates& operator&=(const ExitStates& other) {
				*this = *this & other;
				return *this;
			}
			
			ExitStates& operator|=(const ExitStates& other) {
				*this = *this | other;
				return *this;
			}
			
			bool operator==(const ExitStates& other) const {
				return states_ == other.states_;
			}
			
			bool operator!=(const ExitStates& other) const {
				return states_ != other.states_;
			}
			
		private:
			enum State {
				StateNormal = 0,
				StateReturn = 1,
				StateBreak = 2,
				StateContinue = 3,
				StateThrow = 4,
				StateRethrow = 5
			};
			
			ExitStates()
			: states_(0) { }
			
			explicit ExitStates(const State state)
			: states_(1 << static_cast<unsigned int>(state)) { }
			
			bool test(const unsigned int index) const {
				return ((states_ >> index) & 0x01) == 0x01;
			}
			
			unsigned char states_;
			
		};
		
	}
	
}

#endif
