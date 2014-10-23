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
	
		Statement* Statement::ValueStmt(Value* value) {
			Statement* statement = new Statement(VALUE);
			statement->valueStmt_.value = value;
			return statement;
		}
		
		Statement* Statement::ScopeStmt(std::unique_ptr<Scope> scope) {
			Statement* statement = new Statement(SCOPE);
			statement->scopeStmt_.scope = std::move(scope);
			return statement;
		}
		
		Statement* Statement::InitialiseStmt(Var* var, Value* value) {
			Statement* statement = new Statement(INITIALISE);
			statement->initialiseStmt_.var = var;
			statement->initialiseStmt_.value = value;
			return statement;
		}
		
		Statement* Statement::If(const std::vector<IfClause*>& ifClauses, std::unique_ptr<Scope> elseScope) {
			assert(elseScope != nullptr);
			Statement* statement = new Statement(IF);
			statement->ifStmt_.clauseList = ifClauses;
			statement->ifStmt_.elseScope = std::move(elseScope);
			return statement;
		}
		
		Statement* Statement::Switch(Value* value, const std::vector<SwitchCase*>& caseList, std::unique_ptr<Scope> defaultScope) {
			Statement* statement = new Statement(SWITCH);
			statement->switchStmt_.value = value;
			statement->switchStmt_.caseList = caseList;
			statement->switchStmt_.defaultScope = std::move(defaultScope);
			return statement;
		}
		
		Statement* Statement::Loop(Value* condition, std::unique_ptr<Scope> iterationScope, std::unique_ptr<Scope> advanceScope) {
			Statement* statement = new Statement(LOOP);
			statement->loopStmt_.condition = condition;
			statement->loopStmt_.iterationScope = std::move(iterationScope);
			statement->loopStmt_.advanceScope = std::move(advanceScope);
			return statement;
		}
		
		Statement* Statement::Try(std::unique_ptr<Scope> scope, const std::vector<CatchClause*>& catchList) {
			Statement* statement = new Statement(TRY);
			statement->tryStmt_.scope = std::move(scope);
			statement->tryStmt_.catchList = catchList;
			return statement;
		}
		
		Statement* Statement::ScopeExit(const std::string& state, std::unique_ptr<Scope> scope) {
			Statement* statement = new Statement(SCOPEEXIT);
			statement->scopeExitStmt_.state = state;
			statement->scopeExitStmt_.scope = std::move(scope);
			return statement;
		}
		
		Statement* Statement::ReturnVoid() {
			Statement* statement = new Statement(RETURN);
			statement->returnStmt_.value = NULL;
			return statement;
		}
		
		Statement* Statement::Return(Value* value) {
			Statement* statement = new Statement(RETURN);
			statement->returnStmt_.value = value;
			return statement;
		}
		
		Statement* Statement::Throw(Value* value) {
			Statement* statement = new Statement(THROW);
			statement->throwStmt_.value = value;
			return statement;
		}
		
		Statement* Statement::Rethrow() {
			return new Statement(RETHROW);
		}
		
		Statement* Statement::Break() {
			return new Statement(BREAK);
		}
		
		Statement* Statement::Continue() {
			return new Statement(CONTINUE);
		}
		
		Statement* Statement::Assert(Value* value, const std::string& name) {
			Statement* statement = new Statement(ASSERT);
			statement->assertStmt_.value = value;
			statement->assertStmt_.name = name;
			return statement;
		}
		
		Statement* Statement::Unreachable() {
			return new Statement(UNREACHABLE);
		}
		
		Statement::Statement(Kind k)
			: kind_(k) { }
			
		Statement::Kind Statement::kind() const {
			return kind_;
		}
		
		bool Statement::isValueStatement() const {
			return kind() == VALUE;
		}
		
		Value* Statement::getValue() const {
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
		
		Value* Statement::getInitialiseValue() const {
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
		
		Value* Statement::getSwitchValue() const {
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
		
		Value* Statement::getLoopCondition() const {
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
		
		const std::string& Statement::getScopeExitState() const {
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
		
		Value* Statement::getReturnValue() const {
			assert(isReturnStatement());
			return returnStmt_.value;
		}
		
		bool Statement::isThrowStatement() const {
			return kind() == THROW;
		}
		
		Value* Statement::getThrowValue() const {
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
		
		Value* Statement::getAssertValue() const {
			assert(isAssertStatement());
			return assertStmt_.value;
		}
		
		const std::string& Statement::getAssertName() const {
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
									  valueStmt_.value->toString().c_str());
				}
				
				case SCOPE: {
					return makeString("ScopeStatement(scope: %s)",
									  scopeStmt_.scope->toString().c_str());
				}
				
				case INITIALISE: {
					return makeString("InitialiseStatement(var: %s, value: %s)",
									  initialiseStmt_.var->toString().c_str(),
									  initialiseStmt_.value->toString().c_str());
				}
				
				case IF: {
					return makeString("IfStatement(clauseList: %s, elseScope: %s)",
									  makeArrayString(ifStmt_.clauseList).c_str(),
									  ifStmt_.elseScope->toString().c_str());
				}
				
				case SWITCH: {
					return makeString("SwitchStatement(value: %s, caseList: %s, defaultScope: %s)",
									  switchStmt_.value->toString().c_str(),
									  makeArrayString(switchStmt_.caseList).c_str(),
									  switchStmt_.defaultScope != nullptr ?
									  	switchStmt_.defaultScope->toString().c_str() :
									  	"[NONE]");
				}
				
				case LOOP: {
					return makeString("LoopStatement(condition: %s, iteration: %s, advance: %s)",
									  loopStmt_.condition->toString().c_str(),
									  loopStmt_.iterationScope->toString().c_str(),
									  loopStmt_.advanceScope->toString().c_str());
				}
				
				case TRY: {
					return makeString("TryStatement(scope: %s, catchList: %s)",
									  tryStmt_.scope->toString().c_str(),
									  makeArrayString(tryStmt_.catchList).c_str());
				}
				
				case SCOPEEXIT: {
					return makeString("ScopeExitStatement(state: %s, scope: %s)",
									  getScopeExitState().c_str(),
									  getScopeExitScope().toString().c_str());
				}
				
				case RETURN: {
					return makeString("ReturnStatement(value: %s)",
									  returnStmt_.value == NULL ? "[VOID]" :
									  returnStmt_.value->toString().c_str());
				}
				
				case THROW: {
					return makeString("ThrowStatement(value: %s)",
									  throwStmt_.value->toString().c_str());
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
					return makeString("AssertStatement(value: %s, name: %s)", getAssertValue()->toString().c_str(), getAssertName().c_str());
				}
				
				case UNREACHABLE: {
					return "UnreachableStatement";
				}
			}
			
			throw std::logic_error("Unknown SEM::Statement kind.");
		}
		
	}
	
}

