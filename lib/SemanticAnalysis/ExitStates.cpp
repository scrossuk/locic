#include <bitset>
#include <stdexcept>

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/ExitStates.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		std::bitset<UnwindState_MAX> GetScopeExitStates(SEM::Scope* scope) {
			std::bitset<UnwindState_MAX> exitStates;
			
			const auto& statementList = scope->statements();
			
			if (statementList.empty()) {
				exitStates.set(UnwindStateNormal);
				return exitStates;
			}
			
			// All states that aren't an exception state (e.g. UnwindStateThrow)
			// can be blocked by a scope(success) block that always throws.
			bool isNoThrowBlocked = false;
			
			for (const auto statement: statementList) {
				auto statementExitStates = GetStatementExitStates(statement);
				
				// Block 'normal' exit state until we
				// reach the end of the scope.
				statementExitStates.reset(UnwindStateNormal);
				
				// Also block other no-throw states as necessary.
				if (isNoThrowBlocked) {
					statementExitStates.reset(UnwindStateReturn);
					statementExitStates.reset(UnwindStateBreak);
					statementExitStates.reset(UnwindStateContinue);
				}
				
				exitStates |= statementExitStates;
				
				// Handle scope(success) specially, since these statements can
				// be run in a 'normal' state
				if (statement->kind() == SEM::Statement::SCOPEEXIT && statement->getScopeExitState() == "success") {
					const auto scopeExitStates = GetScopeExitStates(&(statement->getScopeExitScope()));
					if (scopeExitStates.test(UnwindStateThrow)) {
						exitStates.set(UnwindStateThrow);
					}
					
					if (scopeExitStates.test(UnwindStateRethrow)) {
						exitStates.set(UnwindStateRethrow);
					}
					
					if (!scopeExitStates.test(UnwindStateNormal)) {
						// No way to return normally from this scope(success),
						// so all subsequent statements will have no-throw
						// exit states blocked (since they'd be filtered and
						// transferred to throwing states by this statement).
						isNoThrowBlocked = true;
					}
				}
			}
			
			auto lastStatementExitState = GetStatementExitStates(statementList.back());
			
			if (isNoThrowBlocked) {
				lastStatementExitState.reset(UnwindStateNormal);
				lastStatementExitState.reset(UnwindStateReturn);
				lastStatementExitState.reset(UnwindStateBreak);
				lastStatementExitState.reset(UnwindStateContinue);
			}
			
			exitStates |= lastStatementExitState;
			
			return exitStates;
		}
		
		std::bitset<UnwindState_MAX> GetStatementExitStates(SEM::Statement* statement) {
			switch(statement->kind()) {
				case SEM::Statement::VALUE: {
					return GetValueExitStates(statement->getValue());
				}
				case SEM::Statement::SCOPE: {
					return GetScopeExitStates(&(statement->getScope()));
				}
				case SEM::Statement::INITIALISE: {
					return GetValueExitStates(statement->getInitialiseValue());
				}
				case SEM::Statement::IF: {
					std::bitset<UnwindState_MAX> exitStates;
					for (const auto ifClause: statement->getIfClauseList()) {
						const auto conditionExitStates = GetValueExitStates(ifClause->condition());
						
						if (conditionExitStates.test(UnwindStateThrow)) {
							exitStates.set(UnwindStateThrow);
						}
						
						exitStates |= GetScopeExitStates(&(ifClause->scope()));
					}
					
					exitStates |= GetScopeExitStates(&(statement->getIfElseScope()));
					return exitStates;
				}
				case SEM::Statement::SWITCH: {
					std::bitset<UnwindState_MAX> exitStates;
					
					const auto switchValueExitStates = GetValueExitStates(statement->getSwitchValue());
					if (switchValueExitStates.test(UnwindStateThrow)) {
						exitStates.set(UnwindStateThrow);
					}
					
					for (auto switchCase: statement->getSwitchCaseList()) {
						exitStates |= GetScopeExitStates(&(switchCase->scope()));
					}
					
					return exitStates;
				}
				case SEM::Statement::LOOP: {
					std::bitset<UnwindState_MAX> exitStates;
					
					const auto conditionExitStates = GetValueExitStates(statement->getLoopCondition());
					if (conditionExitStates.test(UnwindStateThrow)) {
						exitStates.set(UnwindStateThrow);
					}
					
					auto iterationScopeExitStates = GetScopeExitStates(&(statement->getLoopIterationScope()));
					
					// Block any 'continue' exit state.
					iterationScopeExitStates.reset(UnwindStateContinue);
					
					// A 'break' exit state means a normal return from the loop.
					if (iterationScopeExitStates.test(UnwindStateBreak)) {
						exitStates.set(UnwindStateNormal);
						iterationScopeExitStates.reset(UnwindStateBreak);
					}
					
					exitStates |= iterationScopeExitStates;
					
					auto advanceScopeExitStates = GetScopeExitStates(&(statement->getLoopAdvanceScope()));
					advanceScopeExitStates.reset(UnwindStateNormal);
					exitStates |= advanceScopeExitStates;
					
					return exitStates;
				}
				case SEM::Statement::TRY: {
					std::bitset<UnwindState_MAX> exitStates;
					
					exitStates |= GetScopeExitStates(&(statement->getTryScope()));
					
					for (auto catchClause: statement->getTryCatchList()) {
						auto catchExitStates = GetScopeExitStates(&(catchClause->scope()));
						
						// Turn 'rethrow' into 'throw'.
						if (catchExitStates.test(UnwindStateRethrow)) {
							exitStates.set(UnwindStateThrow);
							catchExitStates.reset(UnwindStateRethrow);
						}
						
						exitStates |= catchExitStates;
					}
					
					return exitStates;
				}
				case SEM::Statement::ASSERT:
				case SEM::Statement::SCOPEEXIT: {
					std::bitset<UnwindState_MAX> exitStates;
					exitStates.set(UnwindStateNormal);
					return exitStates;
				}
				case SEM::Statement::RETURN: {
					std::bitset<UnwindState_MAX> exitStates;
					exitStates.set(UnwindStateReturn);
					return exitStates;
				}
				case SEM::Statement::THROW: {
					std::bitset<UnwindState_MAX> exitStates;
					exitStates.set(UnwindStateThrow);
					return exitStates;
				}
				case SEM::Statement::RETHROW: {
					std::bitset<UnwindState_MAX> exitStates;
					exitStates.set(UnwindStateRethrow);
					return exitStates;
				}
				case SEM::Statement::BREAK: {
					std::bitset<UnwindState_MAX> exitStates;
					exitStates.set(UnwindStateBreak);
					return exitStates;
				}
				case SEM::Statement::CONTINUE: {
					std::bitset<UnwindState_MAX> exitStates;
					exitStates.set(UnwindStateContinue);
					return exitStates;
				}
				case SEM::Statement::UNREACHABLE: {
					return std::bitset<UnwindState_MAX>();
				}
				default: {
					throw std::runtime_error("Unknown statement kind.");
				}
			}
		}
		
		SEM::Type* getFunctionType(SEM::Type* type) {
			if (type->isInterfaceMethod()) {
				return type->getInterfaceMethodFunctionType();
			}
			
			if (type->isMethod()) {
				return type->getMethodFunctionType();
			}
			
			return type;
		}
		
		std::bitset<UnwindState_MAX> GetValueExitStates(SEM::Value* value) {
			switch (value->kind()) {
				case SEM::Value::REINTERPRET: {
					return GetValueExitStates(value->reinterpretValue.value);
				}
				case SEM::Value::DEREF_REFERENCE: {
					return GetValueExitStates(value->derefReference.value);
				}
				case SEM::Value::TERNARY: {
					return GetValueExitStates(value->ternary.condition) |
						GetValueExitStates(value->ternary.ifTrue) |
						GetValueExitStates(value->ternary.ifFalse);
				}
				case SEM::Value::CAST: {
					return GetValueExitStates(value->cast.value);
				}
				case SEM::Value::POLYCAST: {
					return GetValueExitStates(value->polyCast.value);
				}
				case SEM::Value::LVAL: {
					return GetValueExitStates(value->makeLval.value);
				}
				case SEM::Value::NOLVAL: {
					return GetValueExitStates(value->makeNoLval.value);
				}
				case SEM::Value::REF: {
					return GetValueExitStates(value->makeRef.value);
				}
				case SEM::Value::NOREF: {
					return GetValueExitStates(value->makeNoRef.value);
				}
				case SEM::Value::INTERNALCONSTRUCT: {
					std::bitset<UnwindState_MAX> exitStates;
					for (const auto parameter: value->internalConstruct.parameters) {
						exitStates |= GetValueExitStates(parameter);
					}
					return exitStates;
				}
				case SEM::Value::MEMBERACCESS: {
					return GetValueExitStates(value->memberAccess.object);
				}
				case SEM::Value::REFVALUE: {
					return GetValueExitStates(value->refValue.value);
				}
				case SEM::Value::FUNCTIONCALL: {
					std::bitset<UnwindState_MAX> exitStates;
					for (const auto parameter: value->functionCall.parameters) {
						exitStates |= GetValueExitStates(parameter);
					}
					
					const auto functionValue = value->functionCall.functionValue;
					const auto functionType = getFunctionType(functionValue->type());
					assert(functionType->isFunction());
					
					if (!functionType->isFunctionNoExcept()) {
						exitStates.set(UnwindStateThrow);
					}
					
					exitStates |= GetValueExitStates(functionValue);
					
					return exitStates;
				}
				default: {
					std::bitset<UnwindState_MAX> exitStates;
					exitStates.set(UnwindStateNormal);
					return exitStates;
				}
			}
		}
		
	}
	
}


