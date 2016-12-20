#include <locic/AST/CatchClause.hpp>
#include <locic/AST/IfClause.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/Statement.hpp>
#include <locic/AST/SwitchCase.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		Statement* Statement::ValueStmt(Node<ValueDecl> value) {
			Statement* statement = new Statement(VALUE);
			statement->valueStmt.value = std::move(value);
			statement->valueStmt.hasVoidCast = false;
			return statement;
		}
		
		Statement* Statement::ValueStmtVoidCast(Node<ValueDecl> value) {
			Statement* statement = new Statement(VALUE);
			statement->valueStmt.value = std::move(value);
			statement->valueStmt.hasVoidCast = true;
			return statement;
		}
		
		Statement* Statement::ScopeStmt(Node<Scope> scope) {
			Statement* statement = new Statement(SCOPE);
			statement->scopeStmt.scope = std::move(scope);
			return statement;
		}
		
		Statement* Statement::If(Node<IfClauseList> clauseList, Node<Scope> elseScope) {
			Statement* statement = new Statement(IF);
			statement->ifStmt.clauseList = std::move(clauseList);
			statement->ifStmt.elseScope = std::move(elseScope);
			return statement;
		}
		
		Statement*
		Statement::Switch(Node<ValueDecl> value, Node<SwitchCaseList> caseList,
		                  Node<DefaultCase> defaultCase) {
			Statement* statement = new Statement(SWITCH);
			statement->switchStmt.value = std::move(value);
			statement->switchStmt.caseList = std::move(caseList);
			statement->switchStmt.defaultCase = std::move(defaultCase);
			return statement;
		}
		
		Statement*
		Statement::While(Node<ValueDecl> condition, Node<Scope> whileTrue) {
			Statement* statement = new Statement(WHILE);
			statement->whileStmt.condition = std::move(condition);
			statement->whileStmt.whileTrue = std::move(whileTrue);
			return statement;
		}
		
		Statement*
		Statement::For(Node<Var> typeVar, Node<ValueDecl> initValue, Node<Scope> scope) {
			Statement* statement = new Statement(FOR);
			statement->forStmt.typeVar = std::move(typeVar);
			statement->forStmt.initValue = std::move(initValue);
			statement->forStmt.scope = std::move(scope);
			return statement;
		}
		
		Statement*
		Statement::Try(Node<Scope> scope, Node<CatchClauseList> catchList) {
			Statement* statement = new Statement(TRY);
			statement->tryStmt.scope = std::move(scope);
			statement->tryStmt.catchList = std::move(catchList);
			return statement;
		}
		
		Statement*
		Statement::ScopeExit(const String& state, Node<Scope> scope) {
			Statement* statement = new Statement(SCOPEEXIT);
			statement->scopeExitStmt.state = state;
			statement->scopeExitStmt.scope = std::move(scope);
			return statement;
		}
		
		Statement*
		Statement::VarDecl(Node<Var> typeVar, Node<ValueDecl> value) {
			Statement* statement = new Statement(VARDECL);
			statement->varDecl.typeVar = std::move(typeVar);
			statement->varDecl.value = std::move(value);
			return statement;
		}
		
		Statement*
		Statement::Assign(AssignKind assignKind, Node<ValueDecl> var,
		                  Node<ValueDecl> value) {
			Statement* statement = new Statement(ASSIGN);
			statement->assignStmt.assignKind = assignKind;
			statement->assignStmt.var = std::move(var);
			statement->assignStmt.value = std::move(value);
			return statement;
		}
		
		Statement* Statement::Increment(Node<ValueDecl> value) {
			Statement* statement = new Statement(INCREMENT);
			statement->incrementStmt.value = std::move(value);
			return statement;
		}
		
		Statement* Statement::Decrement(Node<ValueDecl> value) {
			Statement* statement = new Statement(DECREMENT);
			statement->decrementStmt.value = std::move(value);
			return statement;
		}
		
		Statement* Statement::Return(Node<ValueDecl> value) {
			Statement* statement = new Statement(RETURN);
			statement->returnStmt.value = std::move(value);
			return statement;
		}
		
		Statement* Statement::ReturnVoid() {
			return new Statement(RETURNVOID);
		}
		
		Statement* Statement::Throw(Node<ValueDecl> value) {
			Statement* statement = new Statement(THROW);
			statement->throwStmt.value = std::move(value);
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
		
		Statement* Statement::Assert(Node<ValueDecl> value, const String& name) {
			Statement* statement = new Statement(ASSERT);
			statement->assertStmt.value = std::move(value);
			statement->assertStmt.name = name;
			return statement;
		}
		
		Statement* Statement::AssertNoExcept(Node<Scope> scope) {
			Statement* statement = new Statement(ASSERTNOEXCEPT);
			statement->assertNoExceptStmt.scope = std::move(scope);
			return statement;
		}
		
		Statement* Statement::Unreachable() {
			return new Statement(UNREACHABLE);
		}
		
		Statement::Statement(const Kind argKind)
		: kind_(argKind) { }
		
		Statement::Kind Statement::kind() const {
			return kind_;
		}
		
		bool Statement::isValue() const {
			return kind() == VALUE;
		}
		
		bool Statement::isUnusedResultValue() const {
			assert(isValue());
			return valueStmt.hasVoidCast;
		}
		
		const Node<ValueDecl>& Statement::value() const {
			assert(isValue());
			return valueStmt.value;
		}
		
		bool Statement::isScope() const {
			return kind() == SCOPE;
		}
		
		Node<Scope>& Statement::scope() {
			assert(isScope());
			return scopeStmt.scope;
		}
		
		const Node<Scope>& Statement::scope() const {
			assert(isScope());
			return scopeStmt.scope;
		}
		
		bool Statement::isIf() const {
			return kind() == IF;
		}
		
		const Node<IfClauseList>& Statement::ifClauseList() const {
			assert(isIf());
			return ifStmt.clauseList;
		}
		
		Node<Scope>& Statement::ifElseScope() {
			assert(isIf());
			return ifStmt.elseScope;
		}
		
		const Node<Scope>& Statement::ifElseScope() const {
			assert(isIf());
			return ifStmt.elseScope;
		}
		
		bool Statement::isSwitch() const {
			return kind() == SWITCH;
		}
		
		const Node<ValueDecl>& Statement::switchValue() const {
			assert(isSwitch());
			return switchStmt.value;
		}
		
		const Node<SwitchCaseList>& Statement::switchCaseList() const {
			assert(isSwitch());
			return switchStmt.caseList;
		}
		
		const Node<DefaultCase>& Statement::defaultCase() const {
			assert(isSwitch());
			return switchStmt.defaultCase;
		}
		
		bool Statement::isWhile() const {
			return kind() == WHILE;
		}
		
		const Node<ValueDecl>& Statement::whileCondition() const {
			assert(isWhile());
			return whileStmt.condition;
		}
		
		Node<Scope>& Statement::whileScope() {
			assert(isWhile());
			return whileStmt.whileTrue;
		}
		
		const Node<Scope>& Statement::whileScope() const {
			assert(isWhile());
			return whileStmt.whileTrue;
		}
		
		bool Statement::isFor() const {
			return kind() == FOR;
		}
		
		Node<Var>& Statement::forVar() {
			return forStmt.typeVar;
		}
		
		const Node<Var>& Statement::forVar() const {
			return forStmt.typeVar;
		}
		
		const Node<ValueDecl>& Statement::forInitValue() const {
			return forStmt.initValue;
		}
		
		Node<Scope>& Statement::forInitScope() {
			return forStmt.scope;
		}
		
		const Node<Scope>& Statement::forInitScope() const {
			return forStmt.scope;
		}
		
		bool Statement::isTry() const {
			return kind() == TRY;
		}
		
		Node<Scope>& Statement::tryScope() {
			assert(isTry());
			return tryStmt.scope;
		}
		
		const Node<Scope>& Statement::tryScope() const {
			assert(isTry());
			return tryStmt.scope;
		}
		
		const Node<CatchClauseList>& Statement::tryCatchList() const {
			assert(isTry());
			return tryStmt.catchList;
		}
		
		bool Statement::isScopeExit() const {
			return kind() == SCOPEEXIT;
		}
		
		const String& Statement::scopeExitState() const {
			assert(isScopeExit());
			return scopeExitStmt.state;
		}
		
		Node<Scope>& Statement::scopeExitScope() {
			assert(isScopeExit());
			return scopeExitStmt.scope;
		}
		
		const Node<Scope>& Statement::scopeExitScope() const {
			assert(isScopeExit());
			return scopeExitStmt.scope;
		}
		
		bool Statement::isVarDecl() const {
			return kind() == VARDECL;
		}
		
		Node<Var>& Statement::varDeclVar() {
			assert(isVarDecl());
			return varDecl.typeVar;
		}
		
		const Node<Var>& Statement::varDeclVar() const {
			assert(isVarDecl());
			return varDecl.typeVar;
		}
		
		const Node<ValueDecl>& Statement::varDeclValue() const {
			assert(isVarDecl());
			return varDecl.value;
		}
		
		bool Statement::isAssign() const {
			return kind() == ASSIGN;
		}
		
		AssignKind Statement::assignKind() const {
			assert(isAssign());
			return assignStmt.assignKind;
		}
		
		const Node<ValueDecl>& Statement::assignLvalue() const {
			assert(isAssign());
			return assignStmt.var;
		}
		
		const Node<ValueDecl>& Statement::assignRvalue() const {
			assert(isAssign());
			return assignStmt.value;
		}
		
		bool Statement::isIncrement() const {
			return kind() == INCREMENT;
		}
		
		const Node<ValueDecl>& Statement::incrementValue() const {
			assert(isIncrement());
			return incrementStmt.value;
		}
		
		bool Statement::isDecrement() const {
			return kind() == DECREMENT;
		}
		
		const Node<ValueDecl>& Statement::decrementValue() const {
			assert(isDecrement());
			return decrementStmt.value;
		}
		
		bool Statement::isReturn() const {
			return kind() == RETURN;
		}
		
		const Node<ValueDecl>& Statement::returnValue() const {
			assert(isReturn());
			return returnStmt.value;
		}
		
		bool Statement::isReturnVoid() const {
			return kind() == RETURNVOID;
		}
		
		bool Statement::isThrow() const {
			return kind() == THROW;
		}
		
		const Node<ValueDecl>& Statement::throwValue() const {
			assert(isThrow());
			return throwStmt.value;
		}
		
		bool Statement::isRethrow() const {
			return kind() == RETHROW;
		}
		
		bool Statement::isBreak() const {
			return kind() == BREAK;
		}
		
		bool Statement::isContinue() const {
			return kind() == CONTINUE;
		}
		
		bool Statement::isAssert() const {
			return kind() == ASSERT;
		}
		
		const Node<ValueDecl>& Statement::assertValue() const {
			assert(isAssert());
			return assertStmt.value;
		}
		
		const String& Statement::assertName() const {
			assert(isAssert());
			return assertStmt.name;
		}
		
		bool Statement::isAssertNoExcept() const {
			return kind() == ASSERTNOEXCEPT;
		}
		
		Node<Scope>& Statement::assertNoExceptScope() {
			assert(isAssertNoExcept());
			return assertNoExceptStmt.scope;
		}
		
		const Node<Scope>& Statement::assertNoExceptScope() const {
			assert(isAssertNoExcept());
			return assertNoExceptStmt.scope;
		}
		
		bool Statement::isUnreachable() const {
			return kind() == UNREACHABLE;
		}
		
	}
	
}

