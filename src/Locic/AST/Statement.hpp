#ifndef LOCIC_AST_STATEMENT_HPP
#define LOCIC_AST_STATEMENT_HPP

#include <string>

#include <Locic/AST/Type.hpp>
#include <Locic/AST/Value.hpp>

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
			Type* type;  // NULL when the keyword 'auto' is used.
			std::string varName;
			Value* value;
		} varDecl;
		
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
		
		inline static Statement* VarDecl(Type* type, const std::string& name, Value* value) {
			Statement* statement = new Statement(VARDECL);
			statement->varDecl.type = type;
			statement->varDecl.varName = name;
			statement->varDecl.value = value;
			return statement;
		}
		
		inline static Statement* AutoVarDecl(const std::string& name, Value* value) {
			Statement* statement = new Statement(VARDECL);
			statement->varDecl.type = 0;
			statement->varDecl.varName = name;
			statement->varDecl.value = value;
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
