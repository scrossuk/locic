#ifndef LOCIC_AST_STATEMENTDECL_HPP
#define LOCIC_AST_STATEMENTDECL_HPP

#include <locic/AST/CatchClause.hpp>
#include <locic/AST/IfClause.hpp>
#include <locic/AST/SwitchCase.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>
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
		
		class StatementDecl {
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
			
			static StatementDecl* ValueStmt(Node<ValueDecl> value);
			
			static StatementDecl* ValueStmtVoidCast(Node<ValueDecl> value);
			
			static StatementDecl* ScopeStmt(Node<Scope> scope);
			
			static StatementDecl* If(Node<IfClauseList> clauseList, Node<Scope> elseScope);
			
			static StatementDecl* Switch(Node<ValueDecl> value, Node<SwitchCaseList> caseList, Node<DefaultCase> defaultCase);
			
			static StatementDecl* While(Node<ValueDecl> condition, Node<Scope> whileTrue);
			
			static StatementDecl* For(Node<Var> typeVar, Node<ValueDecl> initValue, Node<Scope> scope);
			
			static StatementDecl* Try(Node<Scope> scope, Node<CatchClauseList> catchList);
			
			static StatementDecl* ScopeExit(const String& state, Node<Scope> scope);
			
			static StatementDecl* VarDecl(Node<Var> typeVar, Node<ValueDecl> value);
			
			static StatementDecl* Assign(AssignKind assignKind, Node<ValueDecl> var, Node<ValueDecl> value);
			
			static StatementDecl* Increment(Node<ValueDecl> value);
			
			static StatementDecl* Decrement(Node<ValueDecl> value);
			
			static StatementDecl* Return(Node<ValueDecl> value);
			
			static StatementDecl* ReturnVoid();
			
			static StatementDecl* Throw(Node<ValueDecl> value);
			
			static StatementDecl* Rethrow();
			
			static StatementDecl* Break();
			
			static StatementDecl* Continue();
			
			static StatementDecl* Assert(Node<ValueDecl> value, const String& name);
			
			static StatementDecl* AssertNoExcept(Node<Scope> scope);
			
			static StatementDecl* Unreachable();
			
			Kind kind() const;
			
			bool isValue() const;
			
			bool isUnusedResultValue() const;
			
			const Node<ValueDecl>& value() const;
			
			bool isScope() const;
			
			Node<Scope>& scope();
			const Node<Scope>& scope() const;
			
			bool isIf() const;
			
			const Node<IfClauseList>& ifClauseList() const;
			
			Node<Scope>& ifElseScope();
			const Node<Scope>& ifElseScope() const;
			
			bool isSwitch() const;
			
			const Node<ValueDecl>& switchValue() const;
			
			const Node<SwitchCaseList>& switchCaseList() const;
			
			const Node<DefaultCase>& defaultCase() const;
			
			bool isWhile() const;
			
			const Node<ValueDecl>& whileCondition() const;
			
			Node<Scope>& whileScope();
			const Node<Scope>& whileScope() const;
			
			bool isFor() const;
			
			Node<Var>& forVar();
			const Node<Var>& forVar() const;
			
			const Node<ValueDecl>& forInitValue() const;
			
			Node<Scope>& forInitScope();
			const Node<Scope>& forInitScope() const;
			
			bool isTry() const;
			
			Node<Scope>& tryScope();
			const Node<Scope>& tryScope() const;
			
			const Node<CatchClauseList>& tryCatchList() const;
			
			bool isScopeExit() const;
			
			const String& scopeExitState() const;
			
			Node<Scope>& scopeExitScope();
			const Node<Scope>& scopeExitScope() const;
			
			bool isVarDecl() const;
			
			Node<Var>& varDeclVar();
			const Node<Var>& varDeclVar() const;
			
			const Node<ValueDecl>& varDeclValue() const;
			
			bool isAssign() const;
			
			AssignKind assignKind() const;
			
			const Node<ValueDecl>& assignLvalue() const;
			
			const Node<ValueDecl>& assignRvalue() const;
			
			bool isIncrement() const;
			
			const Node<ValueDecl>& incrementValue() const;
			
			bool isDecrement() const;
			
			const Node<ValueDecl>& decrementValue() const;
			
			bool isReturn() const;
			
			const Node<ValueDecl>& returnValue() const;
			
			bool isReturnVoid() const;
			
			bool isThrow() const;
			
			const Node<ValueDecl>& throwValue() const;
			
			bool isRethrow() const;
			
			bool isBreak() const;
			
			bool isContinue() const;
			
			bool isAssert() const;
			
			const Node<ValueDecl>& assertValue() const;
			
			const String& assertName() const;
			
			bool isAssertNoExcept() const;
			
			Node<Scope>& assertNoExceptScope();
			const Node<Scope>& assertNoExceptScope() const;
			
			bool isUnreachable() const;
			
		private:
			StatementDecl(Kind kind);
			
			Kind kind_;
			
			struct {
				Node<ValueDecl> value;
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
				Node<ValueDecl> value;
				Node<SwitchCaseList> caseList;
				Node<DefaultCase> defaultCase;
			} switchStmt;
			
			struct {
				Node<ValueDecl> condition;
				Node<Scope> whileTrue;
			} whileStmt;
			
			struct {
				Node<Var> typeVar;
				Node<ValueDecl> initValue;
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
				Node<ValueDecl> value;
			} varDecl;
			
			struct {
				AssignKind assignKind;
				Node<ValueDecl> var;
				Node<ValueDecl> value;
			} assignStmt;
			
			struct {
				Node<ValueDecl> value;
			} incrementStmt;
			
			struct {
				Node<ValueDecl> value;
			} decrementStmt;
			
			struct {
				Node<ValueDecl> value;
			} returnStmt;
			
			struct {
				Node<ValueDecl> value;
			} throwStmt;
			
			struct {
				Node<ValueDecl> value;
				String name;
			} assertStmt;
			
			struct {
				Node<Scope> scope;
			} assertNoExceptStmt;
			
		};
		
		typedef std::vector<Node<StatementDecl>> StatementDeclList;
		
	}
	
}

#endif
