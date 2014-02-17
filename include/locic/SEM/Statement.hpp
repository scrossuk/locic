#ifndef LOCIC_SEM_STATEMENT_HPP
#define LOCIC_SEM_STATEMENT_HPP

#include <string>
#include <vector>

namespace locic {

	namespace SEM {
	
		class Scope;
		class SwitchCase;
		class Type;
		class Value;
		class Var;
		
		class Statement {
			public:
				enum Kind {
					VALUE,
					SCOPE,
					INITIALISE,
					IF,
					SWITCH,
					WHILE,
					RETURN
				};
				
				static Statement* ValueStmt(Value* value);
				
				static Statement* ScopeStmt(Scope* scope);
				
				static Statement* InitialiseStmt(Var* var, Value* value);
				
				static Statement* If(Value* condition, Scope* ifTrue, Scope* ifFalse);
				
				static Statement* Switch(Value* value, const std::vector<SwitchCase*>& caseList);
				
				static Statement* While(Value* condition, Scope* whileTrue);
				
				static Statement* ReturnVoid();
				
				static Statement* Return(Value* value);
				
				Kind kind() const;
				
				bool isValueStatement() const;
				
				Value* getValue() const;
				
				bool isScope() const;
				
				Scope& getScope() const;
				
				bool isInitialiseStatement() const;
				
				Var* getInitialiseVar() const;
				
				Value* getInitialiseValue() const;
				
				bool isIfStatement() const;
				
				Value* getIfCondition() const;
				
				Scope& getIfTrueScope() const;
				
				bool hasIfFalseScope() const;
				
				Scope& getIfFalseScope() const;
				
				bool isSwitchStatement() const;
				
				Value* getSwitchValue() const;
				
				const std::vector<SwitchCase*>& getSwitchCaseList() const;
				
				bool isWhileStatement() const;
				
				Value* getWhileCondition() const;
				
				Scope& getWhileScope() const;
				
				bool isReturnStatement() const;
				
				Value* getReturnValue() const;
				
				std::string toString() const;
				
			private:
				Statement(Kind k);
					
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
					Value* value;
					std::vector<SwitchCase*> caseList;
				} switchStmt_;
				
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
