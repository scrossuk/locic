#include <assert.h>

#include <string>
#include <vector>

#include <locic/String.hpp>

#include <locic/SEM/CatchClause.hpp>
#include <locic/SEM/IfClause.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Statement.hpp>
#include <locic/SEM/SwitchCase.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Statement* Statement::ValueStmt(Value value) {
			Statement* statement = new Statement(VALUE, value.exitStates());
			statement->valueStmt_.value = std::move(value);
			return statement;
		}
		
		Statement* Statement::ScopeStmt(std::unique_ptr<Scope> scope) {
			Statement* statement = new Statement(SCOPE, scope->exitStates());
			statement->scopeStmt_.scope = std::move(scope);
			return statement;
		}
		
		Statement* Statement::InitialiseStmt(Var* const var, Value value) {
			Statement* statement = new Statement(INITIALISE, value.exitStates());
			statement->initialiseStmt_.var = var;
			statement->initialiseStmt_.value = std::move(value);
			return statement;
		}
		
		Statement* Statement::If(const std::vector<IfClause*>& ifClauses, std::unique_ptr<Scope> elseScope) {
			assert(elseScope != nullptr);
			
			ExitStates exitStates = ExitStates::None();
			
			for (const auto& ifClause: ifClauses) {
				const auto conditionExitStates = ifClause->condition().exitStates();
				
				if (conditionExitStates.hasThrowExit()) {
					exitStates |= ExitStates::Throw();
				}
				
				exitStates |= ifClause->scope().exitStates();
			}
			
			exitStates |= elseScope->exitStates();
			
			Statement* statement = new Statement(IF, exitStates);
			statement->ifStmt_.clauseList = ifClauses;
			statement->ifStmt_.elseScope = std::move(elseScope);
			return statement;
		}
		
		Statement* Statement::Switch(Value value, const std::vector<SwitchCase*>& caseList, std::unique_ptr<Scope> defaultScope) {
			assert(value.type()->isUnionDatatype() || (value.type()->isRef() && value.type()->isBuiltInReference() && value.type()->refTarget()->isUnionDatatype()));
			ExitStates exitStates = ExitStates::None();
			
			const auto switchValueExitStates = value.exitStates();
			if (switchValueExitStates.hasThrowExit()) {
				exitStates |= ExitStates::Throw();
			}
			
			for (const auto& switchCase: caseList) {
				exitStates |= switchCase->scope().exitStates();
			}
			
			if (defaultScope.get() != nullptr) {
				exitStates |= defaultScope->exitStates();
			}
			
			Statement* statement = new Statement(SWITCH, exitStates);
			statement->switchStmt_.value = std::move(value);
			statement->switchStmt_.caseList = caseList;
			statement->switchStmt_.defaultScope = std::move(defaultScope);
			return statement;
		}
		
		Statement* Statement::Loop(Value condition, std::unique_ptr<Scope> iterationScope, std::unique_ptr<Scope> advanceScope) {
			// If the loop condition can be exited normally then the loop
			// can be exited normally (i.e. because the condition can be false).
			ExitStates exitStates = condition.exitStates();
			
			auto iterationScopeExitStates = iterationScope->exitStates();
			
			// Block any 'continue' exit state.
			iterationScopeExitStates &= ~(ExitStates::Continue());
			
			// A 'break' exit state means a normal return from the loop.
			if (iterationScopeExitStates.hasBreakExit()) {
				exitStates |= ExitStates::Normal();
				iterationScopeExitStates &= ~(ExitStates::Break());
			}
			
			exitStates |= iterationScopeExitStates;
			
			auto advanceScopeExitStates = advanceScope->exitStates();
			assert(!advanceScopeExitStates.hasBreakExit() && !advanceScopeExitStates.hasContinueExit());
			
			// Block 'normal' exit from advance scope, since this just
			// goes back to the beginning of the loop.
			advanceScopeExitStates &= ~(ExitStates::Normal());
			
			exitStates |= advanceScopeExitStates;
			
			Statement* statement = new Statement(LOOP, exitStates);
			statement->loopStmt_.condition = std::move(condition);
			statement->loopStmt_.iterationScope = std::move(iterationScope);
			statement->loopStmt_.advanceScope = std::move(advanceScope);
			return statement;
		}
		
		Statement* Statement::Try(std::unique_ptr<Scope> scope, const std::vector<CatchClause*>& catchList) {
			ExitStates exitStates = ExitStates::None();
			
			exitStates |= scope->exitStates();
			
			for (const auto& catchClause: catchList) {
				auto catchExitStates = catchClause->scope().exitStates();
				
				// Turn 'rethrow' into 'throw'.
				if (catchExitStates.hasRethrowExit()) {
					exitStates |= ExitStates::Throw();
					catchExitStates &= ~(ExitStates::Rethrow());
				}
				
				exitStates |= catchExitStates;
			}
			
			Statement* statement = new Statement(TRY, exitStates);
			statement->tryStmt_.scope = std::move(scope);
			statement->tryStmt_.catchList = catchList;
			return statement;
		}
		
		Statement* Statement::ScopeExit(const String& state, std::unique_ptr<Scope> scope) {
			if (state == "exit" || state == "failure") {
				assert((scope->exitStates() & ~(ExitStates::Normal())) == ExitStates::None());
			} else {
				assert((scope->exitStates() & ~(ExitStates::Normal() | ExitStates::Throw() | ExitStates::Rethrow())) == ExitStates::None());
			}
			
			// The exit actions here is for when we first visit this statement,
			// which itself actually has no effect; the effect occurs on unwinding
			// and so this is handled by the owning scope.
			Statement* statement = new Statement(SCOPEEXIT, ExitStates::Normal());
			statement->scopeExitStmt_.state = state;
			statement->scopeExitStmt_.scope = std::move(scope);
			return statement;
		}
		
		Statement* Statement::ReturnVoid() {
			return new Statement(RETURNVOID, ExitStates::Return());
		}
		
		Statement* Statement::Return(Value value) {
			ExitStates exitStates = ExitStates::Return();
			if (value.exitStates().hasThrowExit()) {
				exitStates |= ExitStates::Throw();
			}
			
			Statement* statement = new Statement(RETURN, exitStates);
			statement->returnStmt_.value = std::move(value);
			return statement;
		}
		
		Statement* Statement::Throw(Value value) {
			Statement* statement = new Statement(THROW, ExitStates::Throw());
			statement->throwStmt_.value = std::move(value);
			return statement;
		}
		
		Statement* Statement::Rethrow() {
			return new Statement(RETHROW, ExitStates::Rethrow());
		}
		
		Statement* Statement::Break() {
			return new Statement(BREAK, ExitStates::Break());
		}
		
		Statement* Statement::Continue() {
			return new Statement(CONTINUE, ExitStates::Continue());
		}
		
		Statement* Statement::Assert(Value value, const String& name) {
			Statement* statement = new Statement(ASSERT, value.exitStates());
			statement->assertStmt_.value = std::move(value);
			statement->assertStmt_.name = name;
			return statement;
		}
		
		Statement* Statement::Unreachable() {
			return new Statement(UNREACHABLE, ExitStates::None());
		}
		
		Statement::Statement(const Kind argKind, const ExitStates argExitStates)
		: kind_(argKind), exitStates_(argExitStates) { }
			
		Statement::Kind Statement::kind() const {
			return kind_;
		}
		
		ExitStates Statement::exitStates() const {
			return exitStates_;
		}
		
		bool Statement::isValueStatement() const {
			return kind() == VALUE;
		}
		
		const Value& Statement::getValue() const {
			assert(isValueStatement());
			return valueStmt_.value;
		}
		
		bool Statement::isScope() const {
			return kind() == SCOPE;
		}
		
		Scope& Statement::getScope() const {
			assert(isScope());
			return *(scopeStmt_.scope);
		}
		
		bool Statement::isInitialiseStatement() const {
			return kind() == INITIALISE;
		}
		
		Var* Statement::getInitialiseVar() const {
			assert(isInitialiseStatement());
			return initialiseStmt_.var;
		}
		
		const Value& Statement::getInitialiseValue() const {
			assert(isInitialiseStatement());
			return initialiseStmt_.value;
		}
		
		bool Statement::isIfStatement() const {
			return kind() == IF;
		}
		
		const std::vector<IfClause*>& Statement::getIfClauseList() const {
			assert(isIfStatement());
			return ifStmt_.clauseList;
		}
		
		Scope& Statement::getIfElseScope() const {
			assert(isIfStatement());
			return *(ifStmt_.elseScope);
		}
		
		bool Statement::isSwitchStatement() const {
			return kind() == SWITCH;
		}
		
		const Value& Statement::getSwitchValue() const {
			assert(isSwitchStatement());
			return switchStmt_.value;
		}
		
		const std::vector<SwitchCase*>& Statement::getSwitchCaseList() const {
			assert(isSwitchStatement());
			return switchStmt_.caseList;
		}
		
		Scope* Statement::getSwitchDefaultScope() const {
			assert(isSwitchStatement());
			return switchStmt_.defaultScope.get();
		}
		
		bool Statement::isLoopStatement() const {
			return kind() == LOOP;
		}
		
		const Value& Statement::getLoopCondition() const {
			assert(isLoopStatement());
			return loopStmt_.condition;
		}
		
		Scope& Statement::getLoopIterationScope() const {
			assert(isLoopStatement());
			return *(loopStmt_.iterationScope);
		}
		
		Scope& Statement::getLoopAdvanceScope() const {
			assert(isLoopStatement());
			return *(loopStmt_.advanceScope);
		}
		
		bool Statement::isTryStatement() const {
			return kind() == TRY;
		}
		
		Scope& Statement::getTryScope() const {
			assert(isTryStatement());
			return *(tryStmt_.scope);
		}
		
		const std::vector<CatchClause*>& Statement::getTryCatchList() const {
			assert(isTryStatement());
			return tryStmt_.catchList;
		}
		
		bool Statement::isScopeExitStatement() const {
			return kind() == SCOPEEXIT;
		}
		
		const String& Statement::getScopeExitState() const {
			assert(isScopeExitStatement());
			return scopeExitStmt_.state;
		}
		
		Scope& Statement::getScopeExitScope() const {
			assert(isScopeExitStatement());
			return *(scopeExitStmt_.scope);
		}
		
		bool Statement::isReturnStatement() const {
			return kind() == RETURN;
		}
		
		const Value& Statement::getReturnValue() const {
			assert(isReturnStatement());
			return returnStmt_.value;
		}
		
		bool Statement::isThrowStatement() const {
			return kind() == THROW;
		}
		
		const Value& Statement::getThrowValue() const {
			assert(isThrowStatement());
			return throwStmt_.value;
		}
		
		bool Statement::isRethrowStatement() const {
			return kind() == RETHROW;
		}
		
		bool Statement::isBreakStatement() const {
			return kind() == BREAK;
		}
		
		bool Statement::isContinueStatement() const {
			return kind() == CONTINUE;
		}
		
		bool Statement::isAssertStatement() const {
			return kind() == ASSERT;
		}
		
		const Value& Statement::getAssertValue() const {
			assert(isAssertStatement());
			return assertStmt_.value;
		}
		
		const String& Statement::getAssertName() const {
			assert(isAssertStatement());
			return assertStmt_.name;
		}
		
		bool Statement::isUnreachableStatement() const {
			return kind() == UNREACHABLE;
		}
		
		std::string Statement::toString() const {
			switch (kind_) {
				case VALUE: {
					return makeString("ValueStatement(value: %s)",
						valueStmt_.value.toString().c_str());
				}
				
				case SCOPE: {
					return makeString("ScopeStatement(scope: %s)",
						scopeStmt_.scope->toString().c_str());
				}
				
				case INITIALISE: {
					return makeString("InitialiseStatement(var: %s, value: %s)",
						initialiseStmt_.var->toString().c_str(),
						initialiseStmt_.value.toString().c_str());
				}
				
				case IF: {
					return makeString("IfStatement(clauseList: %s, elseScope: %s)",
						makeArrayPtrString(ifStmt_.clauseList).c_str(),
						ifStmt_.elseScope->toString().c_str());
				}
				
				case SWITCH: {
					return makeString("SwitchStatement(value: %s, caseList: %s, defaultScope: %s)",
						switchStmt_.value.toString().c_str(),
						makeArrayPtrString(switchStmt_.caseList).c_str(),
						switchStmt_.defaultScope != nullptr ?
							switchStmt_.defaultScope->toString().c_str() :
							"[NONE]");
				}
				
				case LOOP: {
					return makeString("LoopStatement(condition: %s, iteration: %s, advance: %s)",
						loopStmt_.condition.toString().c_str(),
						loopStmt_.iterationScope->toString().c_str(),
						loopStmt_.advanceScope->toString().c_str());
				}
				
				case TRY: {
					return makeString("TryStatement(scope: %s, catchList: %s)",
						tryStmt_.scope->toString().c_str(),
						makeArrayPtrString(tryStmt_.catchList).c_str());
				}
				
				case SCOPEEXIT: {
					return makeString("ScopeExitStatement(state: %s, scope: %s)",
						getScopeExitState().c_str(),
						getScopeExitScope().toString().c_str());
				}
				
				case RETURN: {
					return makeString("ReturnStatement(value: %s)",
						returnStmt_.value.toString().c_str());
				}
				
				case RETURNVOID: {
					return "ReturnVoidStatement";
				}
				
				case THROW: {
					return makeString("ThrowStatement(value: %s)",
						throwStmt_.value.toString().c_str());
				}
				
				case RETHROW: {
					return "RethrowStatement";
				}
				
				case BREAK: {
					return "BreakStatement";
				}
				
				case CONTINUE: {
					return "ContinueStatement";
				}
				
				case ASSERT: {
					return makeString("AssertStatement(value: %s, name: %s)",
						getAssertValue().toString().c_str(),
						getAssertName().c_str());
				}
				
				case UNREACHABLE: {
					return "UnreachableStatement";
				}
			}
			
			throw std::logic_error("Unknown SEM::Statement kind.");
		}
		
	}
	
}

