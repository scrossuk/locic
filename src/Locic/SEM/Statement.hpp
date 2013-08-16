#ifndef LOCIC_SEM_STATEMENT_HPP
#define LOCIC_SEM_STATEMENT_HPP

#include <string>

#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/Value.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		class Scope;
		
		class Statement: public Object {
			public:
				enum Kind {
					VALUE,
					SCOPE,
					INITIALISE,
					IF,
					WHILE,
					RETURN
				};
				
				inline static Statement* ValueStmt(Value* value) {
					Statement* statement = new Statement(VALUE);
					statement->valueStmt_.value = value;
					return statement;
				}
				
				inline static Statement* ScopeStmt(Scope* scope) {
					Statement* statement = new Statement(SCOPE);
					statement->scopeStmt_.scope = scope;
					return statement;
				}
				
				inline static Statement* InitialiseStmt(Var* var, Value* value) {
					Statement* statement = new Statement(INITIALISE);
					statement->initialiseStmt_.var = var;
					statement->initialiseStmt_.value = value;
					return statement;
				}
				
				inline static Statement* If(Value* condition, Scope* ifTrue, Scope* ifFalse) {
					Statement* statement = new Statement(IF);
					statement->ifStmt_.condition = condition;
					statement->ifStmt_.ifTrue = ifTrue;
					statement->ifStmt_.ifFalse = ifFalse;
					return statement;
				}
				
				inline static Statement* While(Value* condition, Scope* whileTrue) {
					Statement* statement = new Statement(WHILE);
					statement->whileStmt_.condition = condition;
					statement->whileStmt_.whileTrue = whileTrue;
					return statement;
				}
				
				inline static Statement* ReturnVoid() {
					Statement* statement = new Statement(RETURN);
					statement->returnStmt_.value = NULL;
					return statement;
				}
				
				inline static Statement* Return(Value* value) {
					Statement* statement = new Statement(RETURN);
					statement->returnStmt_.value = value;
					return statement;
				}
				
				inline ObjectKind objectKind() const {
					return OBJECT_STATEMENT;
				}
				
				inline Kind kind() const {
					return kind_;
				}
				
				inline bool isValueStatement() const {
					return kind() == VALUE;
				}
				
				inline Value* getValue() const {
					assert(isValueStatement());
					return valueStmt_.value;
				}
				
				inline bool isScope() const {
					return kind() == SCOPE;
				}
				
				inline Scope& getScope() const {
					assert(isScope());
					return *(scopeStmt_.scope);
				}
				
				inline bool isInitialiseStatement() const {
					return kind() == INITIALISE;
				}
				
				inline Var* getInitialiseVar() const {
					assert(isInitialiseStatement());
					return initialiseStmt_.var;
				}
				
				inline Value* getInitialiseValue() const {
					assert(isInitialiseStatement());
					return initialiseStmt_.value;
				}
				
				inline bool isIfStatement() const {
					return kind() == IF;
				}
				
				inline Value* getIfCondition() const {
					assert(isIfStatement());
					return ifStmt_.condition;
				}
				
				inline Scope& getIfTrueScope() const {
					assert(isIfStatement());
					assert(ifStmt_.ifTrue != NULL);
					return *(ifStmt_.ifTrue);
				}
				
				inline bool hasIfFalseScope() const {
					assert(isIfStatement());
					return ifStmt_.ifFalse != NULL;
				}
				
				inline Scope& getIfFalseScope() const {
					assert(isIfStatement());
					assert(hasIfFalseScope());
					return *(ifStmt_.ifFalse);
				}
				
				inline bool isWhileStatement() const {
					return kind() == WHILE;
				}
				
				inline Value* getWhileCondition() const {
					assert(isWhileStatement());
					return whileStmt_.condition;
				}
				
				inline Scope& getWhileScope() const {
					assert(isWhileStatement());
					return *(whileStmt_.whileTrue);
				}
				
				inline bool isReturnStatement() const {
					return kind() == RETURN;
				}
				
				inline Value* getReturnValue() const {
					assert(isReturnStatement());
					return returnStmt_.value;
				}
				
				std::string toString() const;
				
			private:
				inline Statement(Kind k)
					: kind_(k) { }
					
				Kind kind_;
				
				struct {
					Value* value;
				} valueStmt_;
				
				struct {
					Scope* scope;
				} scopeStmt_;
				
				struct {
					Var* var;
					Value* value;
				} initialiseStmt_;
				
				struct {
					Value* condition;
					Scope* ifTrue, * ifFalse;
				} ifStmt_;
				
				struct {
					Value* condition;
					Scope* whileTrue;
				} whileStmt_;
				
				struct {
					Value* value;
				} returnStmt_;
				
		};
		
	}
	
}

#endif
