#ifndef LOCIC_SEM_STATEMENT_HPP
#define LOCIC_SEM_STATEMENT_HPP

#include <memory>
#include <string>
#include <vector>

#include <locic/AST/ExitStates.hpp>
#include <locic/AST/Value.hpp>
#include <locic/Debug/StatementInfo.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class Type;
		class Var;
		
	}
	
	namespace SEM {
	
		class CatchClause;
		class IfClause;
		class Scope;
		class SwitchCase;
		
		class Statement {
			public:
				enum Kind {
					VALUE,
					SCOPE,
					INITIALISE,
					IF,
					SWITCH,
					LOOP,
					FOR,
					TRY,
					SCOPEEXIT,
					RETURN,
					RETURNVOID,
					THROW,
					RETHROW,
					BREAK,
					CONTINUE,
					ASSERT,
					ASSERTNOEXCEPT,
					UNREACHABLE
				};
				
				static Statement ValueStmt(AST::Value value);
				
				static Statement ScopeStmt(std::unique_ptr<Scope> scope);
				
				static Statement InitialiseStmt(AST::Var& var, AST::Value value);
				
				static Statement If(const std::vector<IfClause*>& ifClauses, std::unique_ptr<Scope> elseScope);
				
				static Statement Switch(AST::Value value, const std::vector<SwitchCase*>& caseList, std::unique_ptr<Scope> defaultScope);
				
				static Statement Loop(AST::Value condition, std::unique_ptr<Scope> iterationScope, std::unique_ptr<Scope> advanceScope);
				
				static Statement For(AST::Var& var, AST::Value initValue,
				                     std::unique_ptr<Scope> scope);
				
				static Statement Try(std::unique_ptr<Scope> scope, const std::vector<CatchClause*>& catchList);
				
				static Statement ScopeExit(const String& state, std::unique_ptr<Scope> scope);
				
				static Statement ReturnVoid();
				
				static Statement Return(AST::Value value);
				
				static Statement Throw(AST::Value value);
				
				static Statement Rethrow();
				
				static Statement Break();
				
				static Statement Continue();
				
				static Statement Assert(AST::Value value, const String& name);
				
				static Statement AssertNoExcept(std::unique_ptr<Scope> scope);
				
				static Statement Unreachable();
				
				Statement(Statement&&) = default;
				Statement& operator=(Statement&&) = default;
				
				Kind kind() const;
				
				AST::ExitStates exitStates() const;
				
				bool isValueStatement() const;
				
				const AST::Value& getValue() const;
				
				bool isScope() const;
				
				Scope& getScope() const;
				
				bool isInitialiseStatement() const;
				
				AST::Var& getInitialiseVar() const;
				
				const AST::Value& getInitialiseValue() const;
				
				bool isIfStatement() const;
				
				const std::vector<IfClause*>& getIfClauseList() const;
				
				Scope& getIfElseScope() const;
				
				bool isSwitchStatement() const;
				
				const AST::Value& getSwitchValue() const;
				
				const std::vector<SwitchCase*>& getSwitchCaseList() const;
				
				Scope* getSwitchDefaultScope() const;
				
				bool isLoopStatement() const;
				
				const AST::Value& getLoopCondition() const;
				
				Scope& getLoopIterationScope() const;
				
				Scope& getLoopAdvanceScope() const;
				
				bool isFor() const;
				
				AST::Var& getForVar() const;
				
				const AST::Value& getForInitValue() const;
				
				Scope& getForScope() const;
				
				bool isTryStatement() const;
				
				Scope& getTryScope() const;
				
				const std::vector<CatchClause*>& getTryCatchList() const;
				
				bool isScopeExitStatement() const;
				
				const String& getScopeExitState() const;
				
				Scope& getScopeExitScope() const;
				
				bool isReturnStatement() const;
				
				const AST::Value& getReturnValue() const;
				
				bool isThrowStatement() const;
				
				const AST::Value& getThrowValue() const;
				
				bool isRethrowStatement() const;
				
				bool isBreakStatement() const;
				
				bool isContinueStatement() const;
				
				bool isAssertStatement() const;
				
				const AST::Value& getAssertValue() const;
				
				const String& getAssertName() const;
				
				bool isAssertNoExceptStatement() const;
				
				const Scope& getAssertNoExceptScope() const;
				
				bool isUnreachableStatement() const;
				
				void setDebugInfo(Debug::StatementInfo debugInfo);
				Optional<Debug::StatementInfo> debugInfo() const;
				
				std::string toString() const;
				
			private:
				Statement(Kind kind, AST::ExitStates exitStates);
				
				Statement(const Statement&) = delete;
				Statement& operator=(const Statement&) = delete;
					
				Kind kind_;
				AST::ExitStates exitStates_;
				Optional<Debug::StatementInfo> debugInfo_;
				
				struct {
					AST::Value value;
				} valueStmt_;
				
				struct {
					std::unique_ptr<Scope> scope;
				} scopeStmt_;
				
				struct {
					AST::Var* var;
					AST::Value value;
				} initialiseStmt_;
				
				struct {
					std::vector<IfClause*> clauseList;
					std::unique_ptr<Scope> elseScope;
				} ifStmt_;
				
				struct {
					AST::Value value;
					std::vector<SwitchCase*> caseList;
					std::unique_ptr<Scope> defaultScope;
				} switchStmt_;
				
				struct {
					AST::Value condition;
					std::unique_ptr<Scope> iterationScope;
					std::unique_ptr<Scope> advanceScope;
				} loopStmt_;
				
				struct {
					AST::Var* var;
					AST::Value initValue;
					std::unique_ptr<Scope> scope;
				} forStmt_;
				
				struct {
					std::unique_ptr<Scope> scope;
					std::vector<CatchClause*> catchList;
				} tryStmt_;
				
				struct {
					String state;
					std::unique_ptr<Scope> scope;
				} scopeExitStmt_;
				
				struct {
					AST::Value value;
				} returnStmt_;
				
				struct {
					AST::Value value;
				} throwStmt_;
				
				struct {
					AST::Value value;
					String name;
				} assertStmt_;
				
				struct {
					std::unique_ptr<Scope> scope;
				} assertNoExceptStmt_;
				
		};
		
	}
	
}

#endif
