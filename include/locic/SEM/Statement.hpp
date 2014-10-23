#ifndef LOCIC_SEM_STATEMENT_HPP
#define LOCIC_SEM_STATEMENT_HPP

#include <memory>
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
					CONTINUE,
					ASSERT,
					UNREACHABLE
				};
				
				static Statement* ValueStmt(Value* value);
				
				static Statement* ScopeStmt(std::unique_ptr<Scope> scope);
				
				static Statement* InitialiseStmt(Var* var, Value* value);
				
				static Statement* If(const std::vector<IfClause*>& ifClauses, std::unique_ptr<Scope> elseScope);
				
				static Statement* Switch(Value* value, const std::vector<SwitchCase*>& caseList, std::unique_ptr<Scope> defaultScope);
				
				static Statement* Loop(Value* condition, std::unique_ptr<Scope> iterationScope, std::unique_ptr<Scope> advanceScope);
				
				static Statement* Try(std::unique_ptr<Scope> scope, const std::vector<CatchClause*>& catchList);
				
				static Statement* ScopeExit(const std::string& state, std::unique_ptr<Scope> scope);
				
				static Statement* ReturnVoid();
				
				static Statement* Return(Value* value);
				
				static Statement* Throw(Value* value);
				
				static Statement* Rethrow();
				
				static Statement* Break();
				
				static Statement* Continue();
				
				static Statement* Assert(Value* value, const std::string& name);
				
				static Statement* Unreachable();
				
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
				
				Scope* getSwitchDefaultScope() const;
				
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
				
				bool isAssertStatement() const;
				
				Value* getAssertValue() const;
				
				const std::string& getAssertName() const;
				
				bool isUnreachableStatement() const;
				
				std::string toString() const;
				
			private:
				Statement(Kind k);
					
				Kind kind_;
				
				struct {
					Value* value;
				} valueStmt_;
				
				struct {
					std::unique_ptr<Scope> scope;
				} scopeStmt_;
				
				struct {
					Var* var;
					Value* value;
				} initialiseStmt_;
				
				struct {
					std::vector<IfClause*> clauseList;
					std::unique_ptr<Scope> elseScope;
				} ifStmt_;
				
				struct {
					Value* value;
					std::vector<SwitchCase*> caseList;
					std::unique_ptr<Scope> defaultScope;
				} switchStmt_;
				
				struct {
					Value* condition;
					std::unique_ptr<Scope> iterationScope;
					std::unique_ptr<Scope> advanceScope;
				} loopStmt_;
				
				struct {
					std::unique_ptr<Scope> scope;
					std::vector<CatchClause*> catchList;
				} tryStmt_;
				
				struct {
					std::string state;
					std::unique_ptr<Scope> scope;
				} scopeExitStmt_;
				
				struct {
					Value* value;
				} returnStmt_;
				
				struct {
					Value* value;
				} throwStmt_;
				
				struct {
					Value* value;
					std::string name;
				} assertStmt_;
				
		};
		
	}
	
}

#endif
