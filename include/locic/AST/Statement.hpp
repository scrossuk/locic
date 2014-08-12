#ifndef LOCIC_AST_STATEMENT_HPP
#define LOCIC_AST_STATEMENT_HPP

#include <string>

#include <locic/AST/CatchClause.hpp>
#include <locic/AST/IfClause.hpp>
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
				FOR,
				TRY,
				SCOPEEXIT,
				VARDECL,
				ASSIGN,
				RETURN,
				RETURNVOID,
				THROW,
				RETHROW,
				BREAK,
				CONTINUE,
				ASSERT,
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
				std::string state;
				Node<Scope> scope;
			} scopeExitStmt;
			
			struct {
				Node<TypeVar> typeVar;
				Node<Value> value;
			} varDecl;
			
			struct {
				Node<Value> value;
			} returnStmt;
			
			struct {
				Node<Value> value;
			} throwStmt;
			
			struct {
				Node<Value> value;
				std::string name;
			} assertStmt;
			
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
			
			inline static Statement* ScopeExit(const std::string& state, const Node<Scope>& scope) {
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
			
			inline static Statement* Assert(const Node<Value>& value, const std::string& name) {
				Statement* statement = new Statement(ASSERT);
				statement->assertStmt.value = value;
				statement->assertStmt.name = name;
				return statement;
			}
			
			inline static Statement* Unreachable() {
				return new Statement(UNREACHABLE);
			}
		};
		
		typedef std::vector<Node<Statement>> StatementList;
		
	}
	
}

#endif
