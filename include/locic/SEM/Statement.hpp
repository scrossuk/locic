#ifndef LOCIC_SEM_STATEMENT_HPP
#define LOCIC_SEM_STATEMENT_HPP

#include <memory>
#include <string>
#include <vector>

#include <locic/SEM/ExitStates.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/String.hpp>

namespace locic {

	namespace SEM {
	
		class CatchClause;
		class IfClause;
		class Scope;
		class SwitchCase;
		class Type;
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
					RETURNVOID,
					THROW,
					RETHROW,
					BREAK,
					CONTINUE,
					ASSERT,
					UNREACHABLE
				};
				
				static Statement* ValueStmt(Value value);
				
				static Statement* ScopeStmt(std::unique_ptr<Scope> scope);
				
				static Statement* InitialiseStmt(Var* var, Value value);
				
				static Statement* If(const std::vector<IfClause*>& ifClauses, std::unique_ptr<Scope> elseScope);
				
				static Statement* Switch(Value value, const std::vector<SwitchCase*>& caseList, std::unique_ptr<Scope> defaultScope);
				
				static Statement* Loop(Value condition, std::unique_ptr<Scope> iterationScope, std::unique_ptr<Scope> advanceScope);
				
				static Statement* Try(std::unique_ptr<Scope> scope, const std::vector<CatchClause*>& catchList);
				
				static Statement* ScopeExit(const String& state, std::unique_ptr<Scope> scope);
				
				static Statement* ReturnVoid();
				
				static Statement* Return(Value value);
				
				static Statement* Throw(Value value);
				
				static Statement* Rethrow();
				
				static Statement* Break();
				
				static Statement* Continue();
				
				static Statement* Assert(Value value, const String& name);
				
				static Statement* Unreachable();
				
				Kind kind() const;
				
				ExitStates exitStates() const;
				
				bool isValueStatement() const;
				
				const Value& getValue() const;
				
				bool isScope() const;
				
				Scope& getScope() const;
				
				bool isInitialiseStatement() const;
				
				Var* getInitialiseVar() const;
				
				const Value& getInitialiseValue() const;
				
				bool isIfStatement() const;
				
				const std::vector<IfClause*>& getIfClauseList() const;
				
				Scope& getIfElseScope() const;
				
				bool isSwitchStatement() const;
				
				const Value& getSwitchValue() const;
				
				const std::vector<SwitchCase*>& getSwitchCaseList() const;
				
				Scope* getSwitchDefaultScope() const;
				
				bool isLoopStatement() const;
				
				const Value& getLoopCondition() const;
				
				Scope& getLoopIterationScope() const;
				
				Scope& getLoopAdvanceScope() const;
				
				bool isTryStatement() const;
				
				Scope& getTryScope() const;
				
				const std::vector<CatchClause*>& getTryCatchList() const;
				
				bool isScopeExitStatement() const;
				
				const String& getScopeExitState() const;
				
				Scope& getScopeExitScope() const;
				
				bool isReturnStatement() const;
				
				const Value& getReturnValue() const;
				
				bool isThrowStatement() const;
				
				const Value& getThrowValue() const;
				
				bool isRethrowStatement() const;
				
				bool isBreakStatement() const;
				
				bool isContinueStatement() const;
				
				bool isAssertStatement() const;
				
				const Value& getAssertValue() const;
				
				const String& getAssertName() const;
				
				bool isUnreachableStatement() const;
				
				std::string toString() const;
				
			private:
				Statement(Kind kind, ExitStates exitStates);
					
				Kind kind_;
				ExitStates exitStates_;
				
				struct {
					Value value;
				} valueStmt_;
				
				struct {
					std::unique_ptr<Scope> scope;
				} scopeStmt_;
				
				struct {
					Var* var;
					Value value;
				} initialiseStmt_;
				
				struct {
					std::vector<IfClause*> clauseList;
					std::unique_ptr<Scope> elseScope;
				} ifStmt_;
				
				struct {
					Value value;
					std::vector<SwitchCase*> caseList;
					std::unique_ptr<Scope> defaultScope;
				} switchStmt_;
				
				struct {
					Value condition;
					std::unique_ptr<Scope> iterationScope;
					std::unique_ptr<Scope> advanceScope;
				} loopStmt_;
				
				struct {
					std::unique_ptr<Scope> scope;
					std::vector<CatchClause*> catchList;
				} tryStmt_;
				
				struct {
					String state;
					std::unique_ptr<Scope> scope;
				} scopeExitStmt_;
				
				struct {
					Value value;
				} returnStmt_;
				
				struct {
					Value value;
				} throwStmt_;
				
				struct {
					Value value;
					String name;
				} assertStmt_;
				
		};
		
	}
	
}

#endif
