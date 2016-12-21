#ifndef LOCIC_AST_STATEMENT_HPP
#define LOCIC_AST_STATEMENT_HPP

#include <memory>
#include <string>
#include <vector>

#include <locic/AST/ExitStates.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Value.hpp>

#include <locic/Debug/StatementInfo.hpp>

#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class CatchClause;
		class DefaultCase;
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
			
			static Statement ValueStmt(Value value);
			
			static Statement ScopeStmt(Node<Scope> scope);
			
			static Statement InitialiseStmt(Var& var, Value value);
			
			static Statement If(const std::vector<IfClause*>& ifClauses,
			                    Node<Scope> elseScope);
			
			static Statement Switch(Value value, const std::vector<SwitchCase*>& caseList,
			                        DefaultCase& defaultCase);
			
			static Statement Loop(Value condition, Node<Scope> iterationScope,
			                      Node<Scope> advanceScope);
			
			static Statement For(Var& var, Value initValue,
			                     Node<Scope> scope);
			
			static Statement Try(Node<Scope> scope,
			                     const std::vector<CatchClause*>& catchList);
			
			static Statement ScopeExit(const String& state,
			                           Node<Scope> scope);
			
			static Statement ReturnVoid();
			
			static Statement Return(Value value);
			
			static Statement Throw(Value value);
			
			static Statement Rethrow();
			
			static Statement Break();
			
			static Statement Continue();
			
			static Statement Assert(Value value, const String& name);
			
			static Statement AssertNoExcept(Node<Scope> scope);
			
			static Statement Unreachable();
			
			Statement(Statement&&) = default;
			Statement& operator=(Statement&&) = default;
			
			Kind kind() const;
			
			ExitStates exitStates() const;
			
			bool isValueStatement() const;
			
			const Value& getValue() const;
			
			bool isScope() const;
			
			Scope& getScope() const;
			
			bool isInitialiseStatement() const;
			
			Var& getInitialiseVar() const;
			
			const Value& getInitialiseValue() const;
			
			bool isIfStatement() const;
			
			const std::vector<IfClause*>&
			getIfClauseList() const;
			
			Scope& getIfElseScope() const;
			
			bool isSwitchStatement() const;
			
			const Value& getSwitchValue() const;
			
			const std::vector<SwitchCase*>&
			getSwitchCaseList() const;
			
			DefaultCase& getSwitchDefaultCase() const;
			
			bool isLoopStatement() const;
			
			const Value& getLoopCondition() const;
			
			Scope& getLoopIterationScope() const;
			
			Scope& getLoopAdvanceScope() const;
			
			bool isFor() const;
			
			Var& getForVar() const;
			
			const Value& getForInitValue() const;
			
			Scope& getForScope() const;
			
			bool isTryStatement() const;
			
			Scope& getTryScope() const;
			
			const std::vector<CatchClause*>&
			getTryCatchList() const;
			
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
			
			bool isAssertNoExceptStatement() const;
			
			const Scope& getAssertNoExceptScope() const;
			
			bool isUnreachableStatement() const;
			
			void setDebugInfo(Debug::StatementInfo debugInfo);
			Optional<Debug::StatementInfo> debugInfo() const;
			
			std::string toString() const;
			
		private:
			Statement(Kind kind, ExitStates exitStates);
			
			Statement(const Statement&) = delete;
			Statement& operator=(const Statement&) = delete;
				
			Kind kind_;
			ExitStates exitStates_;
			Optional<Debug::StatementInfo> debugInfo_;
			
			struct {
				Value value;
			} valueStmt_;
			
			struct {
				Node<Scope> scope;
			} scopeStmt_;
			
			struct {
				Var* var;
				Value value;
			} initialiseStmt_;
			
			struct {
				std::vector<IfClause*> clauseList;
				Node<Scope> elseScope;
			} ifStmt_;
			
			struct {
				Value value;
				std::vector<SwitchCase*> caseList;
				DefaultCase* defaultCase;
			} switchStmt_;
			
			struct {
				Value condition;
				Node<Scope> iterationScope;
				Node<Scope> advanceScope;
			} loopStmt_;
			
			struct {
				Var* var;
				Value initValue;
				Node<Scope> scope;
			} forStmt_;
			
			struct {
				Node<Scope> scope;
				std::vector<CatchClause*> catchList;
			} tryStmt_;
			
			struct {
				String state;
				Node<Scope> scope;
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
			
			struct {
				Node<Scope> scope;
			} assertNoExceptStmt_;
			
		};
		
	}
	
}

#endif
