#ifndef LOCIC_AST_STATEMENT_HPP
#define LOCIC_AST_STATEMENT_HPP

#include <string>

#include <locic/AST/CatchClause.hpp>
#include <locic/AST/SwitchCase.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeVar.hpp>
#include <locic/AST/Value.hpp>

namespace locic {

	namespace AST {
	
		struct Scope;
		
		struct Statement {
			enum TypeEnum {
				NONE,
				VALUE,
				SCOPE,
				IF,
				SWITCH,
				WHILE,
				TRY,
				VARDECL,
				ASSIGN,
				RETURN,
				RETURNVOID,
				THROW,
				BREAK,
				CONTINUE
			} typeEnum;
			
			struct {
				Node<Value> value;
				bool hasVoidCast;
			} valueStmt;
			
			struct {
				Node<Scope> scope;
			} scopeStmt;
			
			struct {
				Node<Value> condition;
				Node<Scope> ifTrue, ifFalse;
			} ifStmt;
			
			struct {
				Node<Value> value;
				Node<SwitchCaseList> caseList;
			} switchStmt;
			
			struct {
				Node<Value> condition;
				Node<Scope> whileTrue;
			} whileStmt;
			
			struct {
				Node<Scope> scope;
				Node<CatchClauseList> catchList;
			} tryStmt;
			
			struct {
				bool isLval;
				Node<TypeVar> typeVar;
				Node<Value> value;
			} varDecl;
			
			struct {
				Node<Value> value;
			} returnStmt;
			
			struct {
				Node<Value> value;
			} throwStmt;
			
			inline Statement()
				: typeEnum(NONE) { }
				
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
			
			inline static Statement* If(const Node<Value>& condition, const Node<Scope>& ifTrue, const Node<Scope>& ifFalse) {
				Statement* statement = new Statement(IF);
				statement->ifStmt.condition = condition;
				statement->ifStmt.ifTrue = ifTrue;
				statement->ifStmt.ifFalse = ifFalse;
				return statement;
			}
			
			inline static Statement* Switch(const Node<Value>& value, const Node<SwitchCaseList>& caseList) {
				Statement* statement = new Statement(SWITCH);
				statement->switchStmt.value = value;
				statement->switchStmt.caseList = caseList;
				return statement;
			}
			
			inline static Statement* While(const Node<Value>& condition, const Node<Scope>& whileTrue) {
				Statement* statement = new Statement(WHILE);
				statement->whileStmt.condition = condition;
				statement->whileStmt.whileTrue = whileTrue;
				return statement;
			}
			
			inline static Statement* Try(const Node<Scope>& scope, const Node<CatchClauseList>& catchList) {
				Statement* statement = new Statement(TRY);
				statement->tryStmt.scope = scope;
				statement->tryStmt.catchList = catchList;
				return statement;
			}
			
			inline static Statement* VarDecl(const Node<TypeVar>& typeVar, const Node<Value>& value) {
				Statement* statement = new Statement(VARDECL);
				statement->varDecl.typeVar = typeVar;
				statement->varDecl.value = value;
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
			
			inline static Statement* Break() {
				return new Statement(BREAK);
			}
			
			inline static Statement* Continue() {
				return new Statement(CONTINUE);
			}
		};
		
		typedef std::vector<Node<Statement>> StatementList;
		
	}
	
}

#endif
