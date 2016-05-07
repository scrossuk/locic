#ifndef LOCIC_AST_STATEMENT_HPP
#define LOCIC_AST_STATEMENT_HPP

#include <locic/AST/CatchClause.hpp>
#include <locic/AST/IfClause.hpp>
#include <locic/AST/SwitchCase.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/Value.hpp>
#include <locic/AST/Var.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		class Scope;
		
		enum AssignKind {
			ASSIGN_DIRECT,
			ASSIGN_ADD,
			ASSIGN_SUB,
			ASSIGN_MUL,
			ASSIGN_DIV,
			ASSIGN_MOD
		};
		
		class Statement {
		public:
			enum Kind {
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
			};
			
			static Statement* ValueStmt(Node<Value> value);
			
			static Statement* ValueStmtVoidCast(Node<Value> value);
			
			static Statement* ScopeStmt(Node<Scope> scope);
			
			static Statement* If(Node<IfClauseList> clauseList, Node<Scope> elseScope);
			
			static Statement* Switch(Node<Value> value, Node<SwitchCaseList> caseList, Node<DefaultCase> defaultCase);
			
			static Statement* While(Node<Value> condition, Node<Scope> whileTrue);
			
			static Statement* For(Node<Var> typeVar, Node<Value> initValue, Node<Scope> scope);
			
			static Statement* Try(Node<Scope> scope, Node<CatchClauseList> catchList);
			
			static Statement* ScopeExit(const String& state, Node<Scope> scope);
			
			static Statement* VarDecl(Node<Var> typeVar, Node<Value> value);
			
			static Statement* Assign(AssignKind assignKind, Node<Value> var, Node<Value> value);
			
			static Statement* Increment(Node<Value> value);
			
			static Statement* Decrement(Node<Value> value);
			
			static Statement* Return(Node<Value> value);
			
			static Statement* ReturnVoid();
			
			static Statement* Throw(Node<Value> value);
			
			static Statement* Rethrow();
			
			static Statement* Break();
			
			static Statement* Continue();
			
			static Statement* Assert(Node<Value> value, const String& name);
			
			static Statement* AssertNoExcept(Node<Scope> scope);
			
			static Statement* Unreachable();
			
			Kind kind() const;
			
			bool isValue() const;
			
			bool isUnusedResultValue() const;
			
			const Node<Value>& value() const;
			
			bool isScope() const;
			
			const Node<Scope>& scope() const;
			
			bool isIf() const;
			
			const Node<IfClauseList>& ifClauseList() const;
			
			const Node<Scope>& ifElseScope() const;
			
			bool isSwitch() const;
			
			const Node<Value>& switchValue() const;
			
			const Node<SwitchCaseList>& switchCaseList() const;
			
			const Node<DefaultCase>& defaultCase() const;
			
			bool isWhile() const;
			
			const Node<Value>& whileCondition() const;
			
			const Node<Scope>& whileScope() const;
			
			bool isFor() const;
			
			Node<Var>& forVar();
			const Node<Var>& forVar() const;
			
			const Node<Value>& forInitValue() const;
			
			const Node<Scope>& forInitScope() const;
			
			bool isTry() const;
			
			const Node<Scope>& tryScope() const;
			
			const Node<CatchClauseList>& tryCatchList() const;
			
			bool isScopeExit() const;
			
			const String& scopeExitState() const;
			
			const Node<Scope>& scopeExitScope() const;
			
			bool isVarDecl() const;
			
			Node<Var>& varDeclVar();
			const Node<Var>& varDeclVar() const;
			
			const Node<Value>& varDeclValue() const;
			
			bool isAssign() const;
			
			AssignKind assignKind() const;
			
			const Node<Value>& assignLvalue() const;
			
			const Node<Value>& assignRvalue() const;
			
			bool isIncrement() const;
			
			const Node<Value>& incrementValue() const;
			
			bool isDecrement() const;
			
			const Node<Value>& decrementValue() const;
			
			bool isReturn() const;
			
			const Node<Value>& returnValue() const;
			
			bool isReturnVoid() const;
			
			bool isThrow() const;
			
			const Node<Value>& throwValue() const;
			
			bool isRethrow() const;
			
			bool isBreak() const;
			
			bool isContinue() const;
			
			bool isAssert() const;
			
			const Node<Value>& assertValue() const;
			
			const String& assertName() const;
			
			bool isAssertNoExcept() const;
			
			const Node<Scope>& assertNoExceptScope() const;
			
			bool isUnreachable() const;
			
		private:
			Statement(Kind kind);
			
			Kind kind_;
			
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
				Node<Var> typeVar;
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
				Node<Var> typeVar;
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
			
		};
		
		typedef std::vector<Node<Statement>> StatementList;
		
	}
	
}

#endif
