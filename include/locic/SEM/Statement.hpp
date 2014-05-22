#ifndef LOCIC_SEM_STATEMENT_HPP
#define LOCIC_SEM_STATEMENT_HPP

#include <string>
#include <vector>

namespace locic {

	namespace SEM {
	
		class CatchClause;
		class IfClause;
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
					LOOP,
					TRY,
					SCOPEEXIT,
					RETURN,
					THROW,
					RETHROW,
					BREAK,
					CONTINUE
				};
				
				static Statement* ValueStmt(Value* value);
				
				static Statement* ScopeStmt(Scope* scope);
				
				static Statement* InitialiseStmt(Var* var, Value* value);
				
				static Statement* If(const std::vector<IfClause*>& ifClauses, Scope* elseScope);
				
				static Statement* Switch(Value* value, const std::vector<SwitchCase*>& caseList);
				
				static Statement* Loop(Value* condition, Scope* iterationScope, Scope* advanceScope);
				
				static Statement* Try(Scope* scope, const std::vector<CatchClause*>& catchList);
				
				static Statement* ScopeExit(const std::string& state, Scope* scope);
				
				static Statement* ReturnVoid();
				
				static Statement* Return(Value* value);
				
				static Statement* Throw(Value* value);
				
				static Statement* Rethrow();
				
				static Statement* Break();
				
				static Statement* Continue();
				
				Kind kind() const;
				
				bool isValueStatement() const;
				
				Value* getValue() const;
				
				bool isScope() const;
				
				Scope& getScope() const;
				
				bool isInitialiseStatement() const;
				
				Var* getInitialiseVar() const;
				
				Value* getInitialiseValue() const;
				
				bool isIfStatement() const;
				
				const std::vector<IfClause*>& getIfClauseList() const;
				
				Scope& getIfElseScope() const;
				
				bool isSwitchStatement() const;
				
				Value* getSwitchValue() const;
				
				const std::vector<SwitchCase*>& getSwitchCaseList() const;
				
				bool isLoopStatement() const;
				
				Value* getLoopCondition() const;
				
				Scope& getLoopIterationScope() const;
				
				Scope& getLoopAdvanceScope() const;
				
				bool isTryStatement() const;
				
				Scope& getTryScope() const;
				
				const std::vector<CatchClause*>& getTryCatchList() const;
				
				bool isScopeExitStatement() const;
				
				const std::string& getScopeExitState() const;
				
				Scope& getScopeExitScope() const;
				
				bool isReturnStatement() const;
				
				Value* getReturnValue() const;
				
				bool isThrowStatement() const;
				
				Value* getThrowValue() const;
				
				bool isRethrowStatement() const;
				
				bool isBreakStatement() const;
				
				bool isContinueStatement() const;
				
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
					std::vector<IfClause*> clauseList;
					Scope* elseScope;
				} ifStmt_;
				
				struct {
					Value* value;
					std::vector<SwitchCase*> caseList;
				} switchStmt_;
				
				struct {
					Value* condition;
					Scope* iterationScope;
					Scope* advanceScope;
				} loopStmt_;
				
				struct {
					Scope* scope;
					std::vector<CatchClause*> catchList;
				} tryStmt_;
				
				struct {
					std::string state;
					Scope* scope;
				} scopeExitStmt_;
				
				struct {
					Value* value;
				} returnStmt_;
				
				struct {
					Value* value;
				} throwStmt_;
				
		};
		
	}
	
}

#endif
