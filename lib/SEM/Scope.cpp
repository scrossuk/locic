#include <string>

#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/ExitStates.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Statement.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		std::unique_ptr<Scope> Scope::Create() {
			return std::unique_ptr<Scope>(new Scope());
		}
		
		Scope::Scope() { }
		
		ExitStates Scope::exitStates() const {
			// TODO: precompute this!
			const auto& statementList = statements();
			
			if (statementList.empty()) {
				return ExitStates::Normal();
			}
			
			ExitStates scopeExitStates = ExitStates::None();
			
			// All states that aren't an exception state (e.g. UnwindStateThrow)
			// can be blocked by a scope(success) block that always throws.
			bool isNoThrowBlocked = false;
			
			// The pending states for scope(success) that occur if we have a no-throw exit.
			ExitStates scopeSuccessPendingStates = ExitStates::None();
			
			for (const auto& statement: statementList) {
				auto statementExitStates = statement.exitStates();
				
				// Block 'normal' exit state until we
				// reach the end of the scope.
				statementExitStates &= ~(ExitStates::Normal());
				
				// Add pending scope(success) exit states if there's
				// a no-throw exit state from this statement (which isn't
				// just continuing to the next statement).
				if ((statementExitStates & (ExitStates::Return() | ExitStates::Break() | ExitStates::Continue())) != ExitStates::None()) {
					scopeExitStates |= scopeSuccessPendingStates;
				}
				
				// Also block other no-throw states as necessary.
				if (isNoThrowBlocked) {
					statementExitStates &= ~(ExitStates::Return() | ExitStates::Break() | ExitStates::Continue());
				}
				
				scopeExitStates |= statementExitStates;
				
				// Handle scope(success) specially, since these statements can
				// be run in a 'normal' state
				if (statement.kind() == SEM::Statement::SCOPEEXIT && statement.getScopeExitState() == "success") {
					const auto scopeSuccessStates = statement.getScopeExitScope().exitStates();
					assert((scopeSuccessStates & ~(ExitStates::Normal() | ExitStates::Throw() | ExitStates::Rethrow())) == ExitStates::None());
					
					if (!scopeSuccessStates.hasNormalExit()) {
						// No way to return normally from this scope(success),
						// so all subsequent statements will have no-throw
						// exit states blocked (since they'd be filtered and
						// transferred to throwing states by this statement).
						isNoThrowBlocked = true;
						
						// Also reset pending scope(success) exit states
						// since any outer scope(success) will never be reached
						// from here because we always throw.
						scopeSuccessPendingStates = ExitStates::None();
					}
					
					// Add throw and rethrow to pending states so that if
					// a no-throw exit state is encountered later then these
					// states can be added.
					if (scopeSuccessStates.hasThrowExit()) {
						scopeSuccessPendingStates |= ExitStates::Throw();
					}
					
					if (scopeSuccessStates.hasRethrowExit()) {
						scopeSuccessPendingStates |= ExitStates::Rethrow();
					}
				}
			}
			
			auto lastStatementExitStates = statementList.back().exitStates();
			
			if ((lastStatementExitStates & (ExitStates::Normal() | ExitStates::Return() | ExitStates::Break() | ExitStates::Continue())) != ExitStates::None()) {
				scopeExitStates |= scopeSuccessPendingStates;
			}
			
			if (isNoThrowBlocked) {
				lastStatementExitStates &= ~(ExitStates::Normal() | ExitStates::Return() | ExitStates::Break() | ExitStates::Continue());
			}
			
			scopeExitStates |= lastStatementExitStates;
			
			return scopeExitStates;
		}
		
		Array<Var*, 10>& Scope::variables() {
			return variables_;
		}
		
		const Array<Var*, 10>& Scope::variables() const {
			return variables_;
		}
		
		FastMap<String, Var*>& Scope::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, Var*>& Scope::namedVariables() const {
			return namedVariables_;
		}
		
		Array<Statement, 10>& Scope::statements() {
			return statementList_;
		}
		
		const Array<Statement, 10>& Scope::statements() const {
			return statementList_;
		}
		
		std::string Scope::toString() const {
			return makeString("Scope(vars: %s, statements: %s)",
					makeArrayPtrString(variables_).c_str(),
					makeArrayString(statementList_).c_str());
		}
		
	}
	
}

