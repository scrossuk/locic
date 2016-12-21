#include <locic/AST/CatchClause.hpp>
#include <locic/AST/IfClause.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/StatementDecl.hpp>
#include <locic/AST/SwitchCase.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		StatementDecl* StatementDecl::ValueStmt(Node<ValueDecl> value) {
			StatementDecl* statement = new StatementDecl(VALUE);
			statement->valueStmt.value = std::move(value);
			statement->valueStmt.hasVoidCast = false;
			return statement;
		}
		
		StatementDecl* StatementDecl::ValueStmtVoidCast(Node<ValueDecl> value) {
			StatementDecl* statement = new StatementDecl(VALUE);
			statement->valueStmt.value = std::move(value);
			statement->valueStmt.hasVoidCast = true;
			return statement;
		}
		
		StatementDecl* StatementDecl::ScopeStmt(Node<Scope> scope) {
			StatementDecl* statement = new StatementDecl(SCOPE);
			statement->scopeStmt.scope = std::move(scope);
			return statement;
		}
		
		StatementDecl* StatementDecl::If(Node<IfClauseList> clauseList, Node<Scope> elseScope) {
			StatementDecl* statement = new StatementDecl(IF);
			statement->ifStmt.clauseList = std::move(clauseList);
			statement->ifStmt.elseScope = std::move(elseScope);
			return statement;
		}
		
		StatementDecl*
		StatementDecl::Switch(Node<ValueDecl> value, Node<SwitchCaseList> caseList,
		                      Node<DefaultCase> defaultCase) {
			StatementDecl* statement = new StatementDecl(SWITCH);
			statement->switchStmt.value = std::move(value);
			statement->switchStmt.caseList = std::move(caseList);
			statement->switchStmt.defaultCase = std::move(defaultCase);
			return statement;
		}
		
		StatementDecl*
		StatementDecl::While(Node<ValueDecl> condition, Node<Scope> whileTrue) {
			StatementDecl* statement = new StatementDecl(WHILE);
			statement->whileStmt.condition = std::move(condition);
			statement->whileStmt.whileTrue = std::move(whileTrue);
			return statement;
		}
		
		StatementDecl*
		StatementDecl::For(Node<Var> typeVar, Node<ValueDecl> initValue, Node<Scope> scope) {
			StatementDecl* statement = new StatementDecl(FOR);
			statement->forStmt.typeVar = std::move(typeVar);
			statement->forStmt.initValue = std::move(initValue);
			statement->forStmt.scope = std::move(scope);
			return statement;
		}
		
		StatementDecl*
		StatementDecl::Try(Node<Scope> scope, Node<CatchClauseList> catchList) {
			StatementDecl* statement = new StatementDecl(TRY);
			statement->tryStmt.scope = std::move(scope);
			statement->tryStmt.catchList = std::move(catchList);
			return statement;
		}
		
		StatementDecl*
		StatementDecl::ScopeExit(const String& state, Node<Scope> scope) {
			StatementDecl* statement = new StatementDecl(SCOPEEXIT);
			statement->scopeExitStmt.state = state;
			statement->scopeExitStmt.scope = std::move(scope);
			return statement;
		}
		
		StatementDecl*
		StatementDecl::VarDecl(Node<Var> typeVar, Node<ValueDecl> value) {
			StatementDecl* statement = new StatementDecl(VARDECL);
			statement->varDecl.typeVar = std::move(typeVar);
			statement->varDecl.value = std::move(value);
			return statement;
		}
		
		StatementDecl*
		StatementDecl::Assign(AssignKind assignKind, Node<ValueDecl> var,
		                      Node<ValueDecl> value) {
			StatementDecl* statement = new StatementDecl(ASSIGN);
			statement->assignStmt.assignKind = assignKind;
			statement->assignStmt.var = std::move(var);
			statement->assignStmt.value = std::move(value);
			return statement;
		}
		
		StatementDecl* StatementDecl::Increment(Node<ValueDecl> value) {
			StatementDecl* statement = new StatementDecl(INCREMENT);
			statement->incrementStmt.value = std::move(value);
			return statement;
		}
		
		StatementDecl* StatementDecl::Decrement(Node<ValueDecl> value) {
			StatementDecl* statement = new StatementDecl(DECREMENT);
			statement->decrementStmt.value = std::move(value);
			return statement;
		}
		
		StatementDecl* StatementDecl::Return(Node<ValueDecl> value) {
			StatementDecl* statement = new StatementDecl(RETURN);
			statement->returnStmt.value = std::move(value);
			return statement;
		}
		
		StatementDecl* StatementDecl::ReturnVoid() {
			return new StatementDecl(RETURNVOID);
		}
		
		StatementDecl* StatementDecl::Throw(Node<ValueDecl> value) {
			StatementDecl* statement = new StatementDecl(THROW);
			statement->throwStmt.value = std::move(value);
			return statement;
		}
		
		StatementDecl* StatementDecl::Rethrow() {
			return new StatementDecl(RETHROW);
		}
		
		StatementDecl* StatementDecl::Break() {
			return new StatementDecl(BREAK);
		}
		
		StatementDecl* StatementDecl::Continue() {
			return new StatementDecl(CONTINUE);
		}
		
		StatementDecl* StatementDecl::Assert(Node<ValueDecl> value, const String& name) {
			StatementDecl* statement = new StatementDecl(ASSERT);
			statement->assertStmt.value = std::move(value);
			statement->assertStmt.name = name;
			return statement;
		}
		
		StatementDecl* StatementDecl::AssertNoExcept(Node<Scope> scope) {
			StatementDecl* statement = new StatementDecl(ASSERTNOEXCEPT);
			statement->assertNoExceptStmt.scope = std::move(scope);
			return statement;
		}
		
		StatementDecl* StatementDecl::Unreachable() {
			return new StatementDecl(UNREACHABLE);
		}
		
		StatementDecl::StatementDecl(const Kind argKind)
		: kind_(argKind) { }
		
		StatementDecl::Kind StatementDecl::kind() const {
			return kind_;
		}
		
		bool StatementDecl::isValue() const {
			return kind() == VALUE;
		}
		
		bool StatementDecl::isUnusedResultValue() const {
			assert(isValue());
			return valueStmt.hasVoidCast;
		}
		
		const Node<ValueDecl>& StatementDecl::value() const {
			assert(isValue());
			return valueStmt.value;
		}
		
		bool StatementDecl::isScope() const {
			return kind() == SCOPE;
		}
		
		Node<Scope>& StatementDecl::scope() {
			assert(isScope());
			return scopeStmt.scope;
		}
		
		const Node<Scope>& StatementDecl::scope() const {
			assert(isScope());
			return scopeStmt.scope;
		}
		
		bool StatementDecl::isIf() const {
			return kind() == IF;
		}
		
		const Node<IfClauseList>& StatementDecl::ifClauseList() const {
			assert(isIf());
			return ifStmt.clauseList;
		}
		
		Node<Scope>& StatementDecl::ifElseScope() {
			assert(isIf());
			return ifStmt.elseScope;
		}
		
		const Node<Scope>& StatementDecl::ifElseScope() const {
			assert(isIf());
			return ifStmt.elseScope;
		}
		
		bool StatementDecl::isSwitch() const {
			return kind() == SWITCH;
		}
		
		const Node<ValueDecl>& StatementDecl::switchValue() const {
			assert(isSwitch());
			return switchStmt.value;
		}
		
		const Node<SwitchCaseList>& StatementDecl::switchCaseList() const {
			assert(isSwitch());
			return switchStmt.caseList;
		}
		
		const Node<DefaultCase>& StatementDecl::defaultCase() const {
			assert(isSwitch());
			return switchStmt.defaultCase;
		}
		
		bool StatementDecl::isWhile() const {
			return kind() == WHILE;
		}
		
		const Node<ValueDecl>& StatementDecl::whileCondition() const {
			assert(isWhile());
			return whileStmt.condition;
		}
		
		Node<Scope>& StatementDecl::whileScope() {
			assert(isWhile());
			return whileStmt.whileTrue;
		}
		
		const Node<Scope>& StatementDecl::whileScope() const {
			assert(isWhile());
			return whileStmt.whileTrue;
		}
		
		bool StatementDecl::isFor() const {
			return kind() == FOR;
		}
		
		Node<Var>& StatementDecl::forVar() {
			return forStmt.typeVar;
		}
		
		const Node<Var>& StatementDecl::forVar() const {
			return forStmt.typeVar;
		}
		
		const Node<ValueDecl>& StatementDecl::forInitValue() const {
			return forStmt.initValue;
		}
		
		Node<Scope>& StatementDecl::forInitScope() {
			return forStmt.scope;
		}
		
		const Node<Scope>& StatementDecl::forInitScope() const {
			return forStmt.scope;
		}
		
		bool StatementDecl::isTry() const {
			return kind() == TRY;
		}
		
		Node<Scope>& StatementDecl::tryScope() {
			assert(isTry());
			return tryStmt.scope;
		}
		
		const Node<Scope>& StatementDecl::tryScope() const {
			assert(isTry());
			return tryStmt.scope;
		}
		
		const Node<CatchClauseList>& StatementDecl::tryCatchList() const {
			assert(isTry());
			return tryStmt.catchList;
		}
		
		bool StatementDecl::isScopeExit() const {
			return kind() == SCOPEEXIT;
		}
		
		const String& StatementDecl::scopeExitState() const {
			assert(isScopeExit());
			return scopeExitStmt.state;
		}
		
		Node<Scope>& StatementDecl::scopeExitScope() {
			assert(isScopeExit());
			return scopeExitStmt.scope;
		}
		
		const Node<Scope>& StatementDecl::scopeExitScope() const {
			assert(isScopeExit());
			return scopeExitStmt.scope;
		}
		
		bool StatementDecl::isVarDecl() const {
			return kind() == VARDECL;
		}
		
		Node<Var>& StatementDecl::varDeclVar() {
			assert(isVarDecl());
			return varDecl.typeVar;
		}
		
		const Node<Var>& StatementDecl::varDeclVar() const {
			assert(isVarDecl());
			return varDecl.typeVar;
		}
		
		const Node<ValueDecl>& StatementDecl::varDeclValue() const {
			assert(isVarDecl());
			return varDecl.value;
		}
		
		bool StatementDecl::isAssign() const {
			return kind() == ASSIGN;
		}
		
		AssignKind StatementDecl::assignKind() const {
			assert(isAssign());
			return assignStmt.assignKind;
		}
		
		const Node<ValueDecl>& StatementDecl::assignLvalue() const {
			assert(isAssign());
			return assignStmt.var;
		}
		
		const Node<ValueDecl>& StatementDecl::assignRvalue() const {
			assert(isAssign());
			return assignStmt.value;
		}
		
		bool StatementDecl::isIncrement() const {
			return kind() == INCREMENT;
		}
		
		const Node<ValueDecl>& StatementDecl::incrementValue() const {
			assert(isIncrement());
			return incrementStmt.value;
		}
		
		bool StatementDecl::isDecrement() const {
			return kind() == DECREMENT;
		}
		
		const Node<ValueDecl>& StatementDecl::decrementValue() const {
			assert(isDecrement());
			return decrementStmt.value;
		}
		
		bool StatementDecl::isReturn() const {
			return kind() == RETURN;
		}
		
		const Node<ValueDecl>& StatementDecl::returnValue() const {
			assert(isReturn());
			return returnStmt.value;
		}
		
		bool StatementDecl::isReturnVoid() const {
			return kind() == RETURNVOID;
		}
		
		bool StatementDecl::isThrow() const {
			return kind() == THROW;
		}
		
		const Node<ValueDecl>& StatementDecl::throwValue() const {
			assert(isThrow());
			return throwStmt.value;
		}
		
		bool StatementDecl::isRethrow() const {
			return kind() == RETHROW;
		}
		
		bool StatementDecl::isBreak() const {
			return kind() == BREAK;
		}
		
		bool StatementDecl::isContinue() const {
			return kind() == CONTINUE;
		}
		
		bool StatementDecl::isAssert() const {
			return kind() == ASSERT;
		}
		
		const Node<ValueDecl>& StatementDecl::assertValue() const {
			assert(isAssert());
			return assertStmt.value;
		}
		
		const String& StatementDecl::assertName() const {
			assert(isAssert());
			return assertStmt.name;
		}
		
		bool StatementDecl::isAssertNoExcept() const {
			return kind() == ASSERTNOEXCEPT;
		}
		
		Node<Scope>& StatementDecl::assertNoExceptScope() {
			assert(isAssertNoExcept());
			return assertNoExceptStmt.scope;
		}
		
		const Node<Scope>& StatementDecl::assertNoExceptScope() const {
			assert(isAssertNoExcept());
			return assertNoExceptStmt.scope;
		}
		
		bool StatementDecl::isUnreachable() const {
			return kind() == UNREACHABLE;
		}
		
	}
	
}

