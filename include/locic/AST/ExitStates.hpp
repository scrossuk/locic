#ifndef LOCIC_SEM_EXITSTATES_HPP
#define LOCIC_SEM_EXITSTATES_HPP

#include <locic/AST/Predicate.hpp>

namespace locic {
	
	namespace AST {
		
		/**
		 * \brief Exit States
		 * 
		 * Every statement and value can be exited in zero
		 * (for the unreachable statement) or more ways;
		 * this class represents the set of states by
		 * which a statement or value can be exited.
		 * 
		 * The include:
		 * 
		 * - 'Normally' (i.e. continue to next statement
		 *               or evaluate outer value).
		 * - Return: Exit the function via a return.
		 * - Break: Exit an enclosing control flow scope.
		 * - Continue: Go to the beginning on an enclosing
		 *             control flow scope.
		 * - Throw: Exit the function via a throw.
		 * - Rethrow: Exit a catch scope via a re-throw.
		 */
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
			
			static ExitStates Throw(Predicate noexceptPredicate) {
				ExitStates exitStates(StateThrow);
				exitStates.noexceptPredicate_ = std::move(noexceptPredicate);
				return exitStates;
			}
			
			static ExitStates ThrowAlways() {
				return ExitStates::Throw(Predicate::False());
			}
			
			static ExitStates Rethrow() {
				return ExitStates(StateRethrow);
			}
			
			static ExitStates AllNonThrowing() {
				return ExitStates::Normal() |
					ExitStates::Return() |
					ExitStates::Break() |
					ExitStates::Continue();
			}
			
			static ExitStates AllThrowing() {
				return ExitStates::ThrowAlways() |
					ExitStates::Rethrow();
			}
			
			static ExitStates NormalAndThrowing() {
				return AllThrowing() | Normal();
			}
			
			static ExitStates AllExceptNormal() {
				return ExitStates::Return() |
					ExitStates::Break() |
					ExitStates::Continue() |
					ExitStates::ThrowAlways() |
					ExitStates::Rethrow();
			}
			
			ExitStates(const ExitStates& other)
			: states_(other.states_),
			noexceptPredicate_(other.noexceptPredicate_.copy()) { }
			
			ExitStates& operator=(const ExitStates& other) {
				states_ = other.states_;
				noexceptPredicate_ = other.noexceptPredicate_.copy();
				return *this;
			}
			
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
			
			const Predicate& noexceptPredicate() const {
				return noexceptPredicate_;
			}
			
			bool hasAnyStates(const ExitStates& other) const {
				return (states_ & other.states_) != 0;
			}
			
			bool onlyHasStates(const ExitStates& other) const {
				return (states_ & other.states_) == states_;
			}
			
			bool onlyHasNormalState() const {
				return onlyHasStates(Normal());
			}
			
			bool onlyHasNormalOrThrowingStates() const {
				return onlyHasStates(NormalAndThrowing());
			}
			
			bool hasAnyNormalOrThrowingStates() const {
				return hasAnyStates(NormalAndThrowing());
			}
			
			bool hasAnyThrowingStates() const {
				return hasAnyStates(AllThrowing());
			}
			
			bool hasAnyNonThrowingStates() const {
				return !onlyHasStates(AllThrowing());
			}
			
			ExitStates throwingStates() const {
				return *this & ExitStates::AllThrowing();
			}
			
			ExitStates operator&(const ExitStates& other) const {
				ExitStates newStates;
				newStates.states_ = states_ & other.states_;
				newStates.noexceptPredicate_ = Predicate::Or(noexceptPredicate_.copy(),
				                                             other.noexceptPredicate_.copy());
				return newStates;
			}
			
			ExitStates operator|(const ExitStates& other) const {
				ExitStates newStates;
				newStates.states_ = states_ | other.states_;
				newStates.noexceptPredicate_ = Predicate::And(noexceptPredicate_.copy(),
				                                              other.noexceptPredicate_.copy());
				return newStates;
			}
			
			void add(const ExitStates& other) {
				*this = *this | other;
			}
			
			void remove(const ExitStates& other) {
				if (hasThrowExit() && other.hasThrowExit()) {
					noexceptPredicate_ = Predicate::True();
				}
				states_ &= ~(other.states_);
			}
			
			void reset() {
				*this = ExitStates::None();
			}
			
			bool operator==(const ExitStates& other) const {
				if (hasThrowExit() &&
				    noexceptPredicate() != other.noexceptPredicate()) {
					return false;
				}
				return states_ == other.states_;
			}
			
			bool operator!=(const ExitStates& other) const {
				return !(*this == other);
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
			: states_(0),
			noexceptPredicate_(Predicate::True()) { }
			
			explicit ExitStates(const State state)
			: states_(static_cast<unsigned char>(1u << static_cast<unsigned int>(state))),
			noexceptPredicate_(Predicate::True()) { }
			
			bool test(const unsigned int index) const {
				return ((states_ >> index) & 0x01) == 0x01;
			}
			
			unsigned char states_;
			Predicate noexceptPredicate_;
			
		};
		
	}
	
}

#endif
