#include <string>

#include <locic/AST/ExitStates.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Statement.hpp>

namespace locic {

	namespace SEM {
	
		std::unique_ptr<Scope> Scope::Create() {
			return std::unique_ptr<Scope>(new Scope());
		}
		
		Scope::Scope() { }
		
		AST::ExitStates Scope::exitStates() const {
			// TODO: precompute this!
			const auto& statementList = statements();
			
			if (statementList.empty()) {
				return AST::ExitStates::Normal();
			}
			
			auto scopeExitStates = AST::ExitStates::None();
			
			// All states that aren't an exception state can be
			// blocked by a scope(success) block that always throws.
			bool isBlockedByAlwaysThrowingScopeSuccess = false;
			
			// The pending states for scope(success) that occur if
			// we have a no-throw exit.
			auto scopeSuccessPendingStates = AST::ExitStates::None();
			
			bool isNormalBlocked = false;
			
			auto lastStatement = &(statementList.back());
			
			for (const auto& statement: statementList) {
				if (isNormalBlocked) {
					break;
				}
				
				auto statementExitStates = statement.exitStates();
				if (!statementExitStates.hasNormalExit()) {
					isNormalBlocked = true;
					lastStatement = &statement;
				}
				
				// Block 'normal' exit state until we
				// reach the end of the scope.
				statementExitStates.remove(AST::ExitStates::Normal());
				
				// Add pending scope(success) exit states if there's
				// a no-throw exit state from this statement (which
				// isn't just continuing to the next statement).
				if (!statementExitStates.onlyHasStates(AST::ExitStates::AllThrowing() | AST::ExitStates::Normal())) {
					scopeExitStates.add(scopeSuccessPendingStates);
				}
				
				// If there's a scope(success) above this code
				// which only throws then we need to block any
				// non-throwing states. For example:
				// 
				// while (true) {
				//     scope (success) {
				//         throw SomeException();
				//     }
				//     break;
				// }
				// 
				// In this case the only way to leave the scope
				// is by throwing an exception.
				if (isBlockedByAlwaysThrowingScopeSuccess) {
					statementExitStates.remove(AST::ExitStates::AllNonThrowing());
				}
				
				assert(!statementExitStates.hasNormalExit());
				scopeExitStates.add(statementExitStates);
				
				// Handle scope(success) specially, since these statements can
				// be run in a 'normal' state
				if (statement.kind() == SEM::Statement::SCOPEEXIT &&
				    statement.getScopeExitState() == "success") {
					const auto scopeSuccessStates = statement.getScopeExitScope().exitStates();
					
					if (!scopeSuccessStates.hasNormalExit()) {
						// No way to return normally from this scope(success),
						// so all subsequent statements will have no-throw
						// exit states blocked (since they'd be filtered and
						// transferred to throwing states by this statement).
						isBlockedByAlwaysThrowingScopeSuccess = true;
						
						// Also reset pending scope(success) exit states
						// since any outer scope(success) will never be reached
						// from here because we always throw.
						scopeSuccessPendingStates.reset();
					}
					
					// Add throw and rethrow to pending states so that if
					// a no-throw exit state is encountered later then these
					// states can be added.
					scopeSuccessPendingStates.add(scopeSuccessStates.throwingStates());
				}
			}
			
			auto lastStatementExitStates = lastStatement->exitStates();
			
			if (lastStatementExitStates.hasAnyNonThrowingStates()) {
				scopeExitStates.add(scopeSuccessPendingStates);
			}
			
			if (isBlockedByAlwaysThrowingScopeSuccess) {
				lastStatementExitStates.remove(AST::ExitStates::AllNonThrowing());
			}
			
			scopeExitStates.add(lastStatementExitStates);
			
			return scopeExitStates;
		}
		
		Array<AST::Var*, 10>& Scope::variables() {
			return variables_;
		}
		
		const Array<AST::Var*, 10>& Scope::variables() const {
			return variables_;
		}
		
		FastMap<String, AST::Var*>& Scope::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, AST::Var*>& Scope::namedVariables() const {
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

