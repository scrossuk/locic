#ifndef LOCIC_SEM_STATEMENT_HPP
#define LOCIC_SEM_STATEMENT_HPP

#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/Value.hpp>
#include <Locic/SEM/Var.hpp>

namespace SEM{

	struct Scope;

	struct Statement {
		enum TypeEnum {
			NONE,
			VALUE,
			SCOPE,
			IF,
			WHILE,
			ASSIGN,
			RETURN
		} typeEnum;
		
		struct {
			Value* value;
		} valueStmt;
		
		struct {
			Scope* scope;
		} scopeStmt;
		
		struct {
			Value* condition;
			Scope* ifTrue, * ifFalse;
		} ifStmt;
		
		struct {
			Value* condition;
			Scope* whileTrue;
		} whileStmt;
		
		struct {
			Value* lValue, * rValue;
		} assignStmt;
		
		struct {
			Value* value;
		} returnStmt;
		
		inline Statement()
			: typeEnum(NONE) { }
			
		inline Statement(TypeEnum e)
			: typeEnum(e) { }
			
		inline static Statement* ValueStmt(Value* value) {
			Statement* statement = new Statement(VALUE);
			statement->valueStmt.value = value;
			return statement;
		}
		
		inline static Statement* ScopeStmt(Scope* scope) {
			Statement* statement = new Statement(SCOPE);
			statement->scopeStmt.scope = scope;
			return statement;
		}
		
		inline static Statement* If(Value* condition, Scope* ifTrue, Scope* ifFalse) {
			Statement* statement = new Statement(IF);
			statement->ifStmt.condition = condition;
			statement->ifStmt.ifTrue = ifTrue;
			statement->ifStmt.ifFalse = ifFalse;
			return statement;
		}
		
		inline static Statement* While(Value* condition, Scope* whileTrue) {
			Statement* statement = new Statement(WHILE);
			statement->whileStmt.condition = condition;
			statement->whileStmt.whileTrue = whileTrue;
			return statement;
		}
		
		inline static Statement* Assign(Value* lValue, Value* rValue) {
			Statement* statement = new Statement(ASSIGN);
			statement->assignStmt.lValue = lValue;
			statement->assignStmt.rValue = rValue;
			return statement;
		}
		
		inline static Statement* ReturnVoid() {
			Statement* statement = new Statement(RETURN);
			statement->returnStmt.value = NULL;
			return statement;
		}
		
		inline static Statement* Return(Value* value) {
			Statement* statement = new Statement(RETURN);
			statement->returnStmt.value = value;
			return statement;
		}
	};

}

#endif
