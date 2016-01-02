#ifndef LOCIC_AST_STATEMENT_HPP
#define LOCIC_AST_STATEMENT_HPP

#include <locic/AST/CatchClause.hpp>
#include <locic/AST/IfClause.hpp>
#include <locic/AST/SwitchCase.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeVar.hpp>
#include <locic/AST/Value.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		struct Scope;
		
		enum AssignKind {
			ASSIGN_DIRECT,
			ASSIGN_ADD,
			ASSIGN_SUB,
			ASSIGN_MUL,
			ASSIGN_DIV,
			ASSIGN_MOD
		};
		
		struct Statement {
			enum TypeEnum {
				VALUE,
				SCOPE,
				IF,
				SWITCH,
				WHILE,
				FOR,
				TRY,
				SCOPEEXIT,
				VARDECL,
				ASSIGN,
				INCREMENT,
				DECREMENT,
				RETURN,
				RETURNVOID,
				THROW,
				RETHROW,
				BREAK,
				CONTINUE,
				ASSERT,
				ASSERTNOEXCEPT,
				UNREACHABLE
			} typeEnum;
			
			struct {
				Node<Value> value;
				bool hasVoidCast;
			} valueStmt;
			
			struct {
				Node<Scope> scope;
			} scopeStmt;
			
			struct {
				Node<IfClauseList> clauseList;
				Node<Scope> elseScope;
			} ifStmt;
			
			struct {
				Node<Value> value;
				Node<SwitchCaseList> caseList;
				Node<DefaultCase> defaultCase;
			} switchStmt;
			
			struct {
				Node<Value> condition;
				Node<Scope> whileTrue;
			} whileStmt;
			
			struct {
				Node<TypeVar> typeVar;
				Node<Value> initValue;
				Node<Scope> scope;
			} forStmt;
			
			struct {
				Node<Scope> scope;
				Node<CatchClauseList> catchList;
			} tryStmt;
			
			struct {
				String state;
				Node<Scope> scope;
			} scopeExitStmt;
			
			struct {
				Node<TypeVar> typeVar;
				Node<Value> value;
			} varDecl;
			
			struct {
				AssignKind assignKind;
				Node<Value> var;
				Node<Value> value;
			} assignStmt;
			
			struct {
				Node<Value> value;
			} incrementStmt;
			
			struct {
				Node<Value> value;
			} decrementStmt;
			
			struct {
				Node<Value> value;
			} returnStmt;
			
			struct {
				Node<Value> value;
			} throwStmt;
			
			struct {
				Node<Value> value;
				String name;
			} assertStmt;
			
			struct {
				Node<Scope> scope;
			} assertNoExceptStmt;
				
			inline Statement(TypeEnum e)
				: typeEnum(e) { }
				
			inline static Statement* ValueStmt(const Node<Value>& value) {
				Statement* statement = new Statement(VALUE);
				statement->valueStmt.value = value;
				statement->valueStmt.hasVoidCast = false;
				return statement;
			}
			
			inline static Statement* ValueStmtVoidCast(const Node<Value>& value) {
				Statement* statement = new Statement(VALUE);
				statement->valueStmt.value = value;
				statement->valueStmt.hasVoidCast = true;
				return statement;
			}
			
			inline static Statement* ScopeStmt(const Node<Scope>& scope) {
				Statement* statement = new Statement(SCOPE);
				statement->scopeStmt.scope = scope;
				return statement;
			}
			
			inline static Statement* If(const Node<IfClauseList>& clauseList, const Node<Scope>& elseScope) {
				Statement* statement = new Statement(IF);
				statement->ifStmt.clauseList = clauseList;
				statement->ifStmt.elseScope = elseScope;
				return statement;
			}
			
			inline static Statement* Switch(const Node<Value>& value, const Node<SwitchCaseList>& caseList, const Node<DefaultCase>& defaultCase) {
				Statement* statement = new Statement(SWITCH);
				statement->switchStmt.value = value;
				statement->switchStmt.caseList = caseList;
				statement->switchStmt.defaultCase = defaultCase;
				return statement;
			}
			
			inline static Statement* While(const Node<Value>& condition, const Node<Scope>& whileTrue) {
				Statement* statement = new Statement(WHILE);
				statement->whileStmt.condition = condition;
				statement->whileStmt.whileTrue = whileTrue;
				return statement;
			}
			
			inline static Statement* For(const Node<TypeVar>& typeVar, const Node<Value>& initValue, const Node<Scope>& scope) {
				Statement* statement = new Statement(FOR);
				statement->forStmt.typeVar = typeVar;
				statement->forStmt.initValue = initValue;
				statement->forStmt.scope = scope;
				return statement;
			}
			
			inline static Statement* Try(const Node<Scope>& scope, const Node<CatchClauseList>& catchList) {
				Statement* statement = new Statement(TRY);
				statement->tryStmt.scope = scope;
				statement->tryStmt.catchList = catchList;
				return statement;
			}
			
			inline static Statement* ScopeExit(const String& state, const Node<Scope>& scope) {
				Statement* statement = new Statement(SCOPEEXIT);
				statement->scopeExitStmt.state = state;
				statement->scopeExitStmt.scope = scope;
				return statement;
			}
			
			inline static Statement* VarDecl(const Node<TypeVar>& typeVar, const Node<Value>& value) {
				Statement* statement = new Statement(VARDECL);
				statement->varDecl.typeVar = typeVar;
				statement->varDecl.value = value;
				return statement;
			}
			
			inline static Statement* Assign(AssignKind assignKind, const Node<Value>& var, const Node<Value>& value) {
				Statement* statement = new Statement(ASSIGN);
				statement->assignStmt.assignKind = assignKind;
				statement->assignStmt.var = var;
				statement->assignStmt.value = value;
				return statement;
			}
			
			inline static Statement* Increment(const Node<Value>& value) {
				Statement* statement = new Statement(INCREMENT);
				statement->incrementStmt.value = value;
				return statement;
			}
			
			inline static Statement* Decrement(const Node<Value>& value) {
				Statement* statement = new Statement(DECREMENT);
				statement->decrementStmt.value = value;
				return statement;
			}
			
			inline static Statement* Return(const Node<Value>& value) {
				Statement* statement = new Statement(RETURN);
				statement->returnStmt.value = value;
				return statement;
			}
			
			inline static Statement* ReturnVoid() {
				return new Statement(RETURNVOID);
			}
			
			inline static Statement* Throw(const Node<Value>& value) {
				Statement* statement = new Statement(THROW);
				statement->throwStmt.value = value;
				return statement;
			}
			
			inline static Statement* Rethrow() {
				return new Statement(RETHROW);
			}
			
			inline static Statement* Break() {
				return new Statement(BREAK);
			}
			
			inline static Statement* Continue() {
				return new Statement(CONTINUE);
			}
			
			inline static Statement* Assert(const Node<Value>& value, const String& name) {
				Statement* statement = new Statement(ASSERT);
				statement->assertStmt.value = value;
				statement->assertStmt.name = name;
				return statement;
			}
			
			inline static Statement* AssertNoExcept(const Node<Scope>& scope) {
				Statement* statement = new Statement(ASSERTNOEXCEPT);
				statement->assertNoExceptStmt.scope = scope;
				return statement;
			}
			
			inline static Statement* Unreachable() {
				return new Statement(UNREACHABLE);
			}
			
			TypeEnum kind() const {
				return typeEnum;
			}
			
			bool isValue() const {
				return kind() == VALUE;
			}
			
			const Node<Value>& value() const {
				assert(isValue());
				return valueStmt.value;
			}
			
			bool isIf() const {
				return kind() == IF;
			}
			
			const Node<IfClauseList>& ifClauseList() const {
				assert(isIf());
				return ifStmt.clauseList;
			}
			
			const Node<Scope>& ifElseScope() const {
				assert(isIf());
				return ifStmt.elseScope;
			}
			
			bool isSwitch() const {
				return kind() == SWITCH;
			}
			
			const Node<Value>& switchValue() const {
				assert(isSwitch());
				return switchStmt.value;
			}
			
			const Node<SwitchCaseList>& switchCaseList() const {
				assert(isSwitch());
				return switchStmt.caseList;
			}
			
			const Node<DefaultCase>& defaultCase() const {
				assert(isSwitch());
				return switchStmt.defaultCase;
			}
			
			bool isWhile() const {
				return kind() == WHILE;
			}
			
			const Node<Value>& whileCondition() const {
				assert(isWhile());
				return whileStmt.condition;
			}
			
			const Node<Scope>& whileScope() const {
				assert(isWhile());
				return whileStmt.whileTrue;
			}
			
			bool isFor() const {
				return kind() == FOR;
			}
			
			const Node<TypeVar>& forTypeVar() const {
				return forStmt.typeVar;
			}
			
			const Node<Value>& forInitValue() const {
				return forStmt.initValue;
			}
			
			const Node<Scope>& forInitScope() const {
				return forStmt.scope;
			}
			
			bool isTry() const {
				return kind() == TRY;
			}
			
			const Node<Scope>& tryScope() const {
				assert(isTry());
				return tryStmt.scope;
			}
			
			const Node<CatchClauseList>& tryCatchList() const {
				assert(isTry());
				return tryStmt.catchList;
			}
			
			bool isScopeExit() const {
				return kind() == SCOPEEXIT;
			}
			
			const String& scopeExitState() const {
				assert(isScopeExit());
				return scopeExitStmt.state;
			}
			
			const Node<Scope>& scopeExitScope() const {
				assert(isScopeExit());
				return scopeExitStmt.scope;
			}
			
			bool isVarDecl() const {
				return kind() == VARDECL;
			}
			
			const Node<TypeVar>& varDeclVar() const {
				assert(isVarDecl());
				return varDecl.typeVar;
			}
			
			const Node<Value>& varDeclValue() const {
				assert(isVarDecl());
				return varDecl.value;
			}
			
			bool isAssign() const {
				return kind() == ASSIGN;
			}
			
			AssignKind assignKind() const {
				assert(isAssign());
				return assignStmt.assignKind;
			}
			
			const Node<Value>& assignLvalue() const {
				assert(isAssign());
				return assignStmt.var;
			}
			
			const Node<Value>& assignRvalue() const {
				assert(isAssign());
				return assignStmt.value;
			}
			
			bool isIncrement() const {
				return kind() == INCREMENT;
			}
			
			const Node<Value>& incrementValue() const {
				assert(isIncrement());
				return incrementStmt.value;
			}
			
			bool isDecrement() const {
				return kind() == DECREMENT;
			}
			
			const Node<Value>& decrementValue() const {
				assert(isDecrement());
				return decrementStmt.value;
			}
			
			bool isReturn() const {
				return kind() == RETURN;
			}
			
			const Node<Value>& returnValue() const {
				assert(isReturn());
				return returnStmt.value;
			}
			
			bool isReturnVoid() const {
				return kind() == RETURNVOID;
			}
			
			bool isThrow() const {
				return kind() == THROW;
			}
			
			const Node<Value>& throwValue() const {
				assert(isThrow());
				return throwStmt.value;
			}
			
			bool isRethrow() const {
				return kind() == RETHROW;
			}
			
			bool isBreak() const {
				return kind() == BREAK;
			}
			
			bool isContinue() const {
				return kind() == CONTINUE;
			}
			
			bool isAssert() const {
				return kind() == ASSERT;
			}
			
			const Node<Value>& assertValue() const {
				assert(isAssert());
				return assertStmt.value;
			}
			
			const String& assertName() const {
				assert(isAssert());
				return assertStmt.name;
			}
			
			bool isAssertNoExcept() const {
				return kind() == ASSERTNOEXCEPT;
			}
			
			const Node<Scope>& assertNoExceptScope() const {
				assert(isAssertNoExcept());
				return assertNoExceptStmt.scope;
			}
			
			bool isUnreachable() const {
				return kind() == UNREACHABLE;
			}
		};
		
		typedef std::vector<Node<Statement>> StatementList;
		
	}
	
}

#endif
