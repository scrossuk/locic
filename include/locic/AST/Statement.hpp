#ifndef LOCIC_AST_STATEMENT_HPP
#define LOCIC_AST_STATEMENT_HPP

#include <string>

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
				WHILE,
				VARDECL,
				ASSIGN,
				RETURN,
				RETURNVOID
			} typeEnum;
			
			struct {
				Node<Value> value;
			} valueStmt;
			
			struct {
				Node<Scope> scope;
			} scopeStmt;
			
			struct {
				Node<Value> condition;
				Node<Scope> ifTrue, ifFalse;
			} ifStmt;
			
			struct {
				Node<Value> condition;
				Node<Scope> whileTrue;
			} whileStmt;
			
			struct {
				bool isLval;
				Node<TypeVar> typeVar;
				Node<Value> value;
			} varDecl;
			
			struct {
				Node<Value> value;
			} returnStmt;
			
			inline Statement()
				: typeEnum(NONE) { }
				
			inline Statement(TypeEnum e)
				: typeEnum(e) { }
				
			inline static Statement* ValueStmt(Node<Value> value) {
				Statement* statement = new Statement(VALUE);
				statement->valueStmt.value = value;
				return statement;
			}
			
			inline static Statement* ScopeStmt(Node<Scope> scope) {
				Statement* statement = new Statement(SCOPE);
				statement->scopeStmt.scope = scope;
				return statement;
			}
			
			inline static Statement* If(Node<Value> condition, Node<Scope> ifTrue, Node<Scope> ifFalse) {
				Statement* statement = new Statement(IF);
				statement->ifStmt.condition = condition;
				statement->ifStmt.ifTrue = ifTrue;
				statement->ifStmt.ifFalse = ifFalse;
				return statement;
			}
			
			inline static Statement* While(Node<Value> condition, Node<Scope> whileTrue) {
				Statement* statement = new Statement(WHILE);
				statement->whileStmt.condition = condition;
				statement->whileStmt.whileTrue = whileTrue;
				return statement;
			}
			
			inline static Statement* VarDecl(Node<TypeVar> typeVar, Node<Value> value) {
				Statement* statement = new Statement(VARDECL);
				statement->varDecl.typeVar = typeVar;
				statement->varDecl.value = value;
				return statement;
			}
			
			inline static Statement* Return(Node<Value> value) {
				Statement* statement = new Statement(RETURN);
				statement->returnStmt.value = value;
				return statement;
			}
			
			inline static Statement* ReturnVoid() {
				return new Statement(RETURNVOID);
			}
		};
		
		typedef std::vector<Node<Statement>> StatementList;
		
	}
	
}

#endif
