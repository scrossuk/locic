#include <cassert>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/Debug/Module.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void DeadCodeSearchScope(Context& context, const SEM::Scope& scope);
		
		void DeadCodeSearchStatement(Context& context, const SEM::Statement& statement) {
			switch(statement.kind()) {
				case SEM::Statement::VALUE:
				case SEM::Statement::INITIALISE:
				case SEM::Statement::RETURN:
				case SEM::Statement::RETURNVOID:
				case SEM::Statement::THROW:
				case SEM::Statement::RETHROW:
				case SEM::Statement::BREAK:
				case SEM::Statement::CONTINUE:
				case SEM::Statement::ASSERT:
				case SEM::Statement::UNREACHABLE: {
					return;
				}
				case SEM::Statement::SCOPE: {
					DeadCodeSearchScope(context, statement.getScope());
					return;
				}
				case SEM::Statement::IF: {
					for (const auto ifClause: statement.getIfClauseList()) {
						DeadCodeSearchScope(context, ifClause->scope());
					}
					
					DeadCodeSearchScope(context, statement.getIfElseScope());
					return;
				}
				case SEM::Statement::SWITCH: {
					for (auto switchCase: statement.getSwitchCaseList()) {
						DeadCodeSearchScope(context, switchCase->scope());
					}
					return;
				}
				case SEM::Statement::LOOP: {
					DeadCodeSearchScope(context, statement.getLoopIterationScope());
					DeadCodeSearchScope(context, statement.getLoopAdvanceScope());
					return;
				}
				case SEM::Statement::TRY: {
					DeadCodeSearchScope(context, statement.getTryScope());
					for (auto catchClause: statement.getTryCatchList()) {
						DeadCodeSearchScope(context, catchClause->scope());
					}
					return;
				}
				case SEM::Statement::SCOPEEXIT: {
					DeadCodeSearchScope(context, statement.getScopeExitScope());
					return;
				}
				case SEM::Statement::ASSERTNOEXCEPT: {
					DeadCodeSearchScope(context, statement.getAssertNoExceptScope());
					return;
				}
			}
			
			std::terminate();
		}
		
		void DeadCodeSearchScope(Context& context, const SEM::Scope& scope) {
			const SEM::Statement* lastScopeExit = nullptr;
			const SEM::Statement* lastScopeFailure = nullptr;
			const SEM::Statement* lastScopeSuccess = nullptr;
			
			bool isNormalBlocked = false;
			for (const auto& statement: scope.statements()) {
				DeadCodeSearchStatement(context, statement);
				
				if (isNormalBlocked) {
					const auto debugInfo = statement.debugInfo();
					assert(debugInfo);
					throw ErrorException(makeString("Function contains dead code beginning with statement at position %s.",
						debugInfo->location.toString().c_str()));
				}
				
				if (statement.isScopeExitStatement()) {
					if (statement.getScopeExitState() == "exit") {
						lastScopeExit = &statement;
					} else if (statement.getScopeExitState() == "failure") {
						lastScopeFailure = &statement;
						
						// Any previous scope(exit) will
						// be reached after this
						// scope(failure) has executed.
						lastScopeExit = nullptr;
					} else if (statement.getScopeExitState() == "success") {
						const auto scopeExitStates = statement.getScopeExitScope().exitStates();
						
						if (!scopeExitStates.hasNormalExit()) {
							// Any previous scope(failure) will
							// be reached after this
							// scope(success) has executed
							// since we're guaranteeed to
							// be throwing.
							lastScopeFailure = nullptr;
						}
						
						if (lastScopeSuccess != nullptr &&
						    !scopeExitStates.hasNormalExit()) {
							throw ErrorException(makeString("scope(success) is unreachable, at position %s, due to later always throwing scope(success), at position %s.",
								lastScopeSuccess->debugInfo()->location.toString().c_str(),
								statement.debugInfo()->location.toString().c_str()));
						}
						
						lastScopeSuccess = &statement;
						
						// Any previous scope(exit) will
						// be reached after this
						// scope(success) has executed.
						lastScopeExit = nullptr;
					} else {
						throw std::logic_error("Unknown scope exit state.");
					}
				}
				
				const auto exitStates = statement.exitStates();
				if (!exitStates.hasNormalExit()) {
					isNormalBlocked = true;
				}
				
				if (exitStates.hasAnyStates(SEM::ExitStates::AllExceptNormal())) {
					// TODO: only the most recent
					// scope(exit) will be reached!
					
					// Scope exits will be reached by
					// anything other than continuing to the
					// next statement, such as an exception
					// being thrown.
					lastScopeExit = nullptr;
					
					if (exitStates.hasAnyNonThrowingStates()) {
						lastScopeSuccess = nullptr;
					}
					
					if (exitStates.hasAnyThrowingStates()) {
						lastScopeFailure = nullptr;
					}
				}
			}
			
			if (isNormalBlocked && lastScopeExit != nullptr) {
				throw ErrorException(makeString("scope(exit) is unreachable, at position %s.",
					lastScopeExit->debugInfo()->location.toString().c_str()));
			}
			
			if (lastScopeFailure != nullptr) {
				throw ErrorException(makeString("scope(failure) is unreachable, at position %s.",
					lastScopeFailure->debugInfo()->location.toString().c_str()));
			}
			
			if (isNormalBlocked && lastScopeSuccess != nullptr) {
				throw ErrorException(makeString("scope(success) is unreachable, at position %s.",
					lastScopeSuccess->debugInfo()->location.toString().c_str()));
			}
		}
		
		void ConvertFunctionDef(Context& context, const AST::Node<AST::Function>& astFunctionNode) {
			const auto semFunction = context.scopeStack().back().function();
			
			// Function should currently be a declaration
			// (it is about to be made into a definition).
			assert(semFunction->isDeclaration());
			
			const auto functionType = semFunction->type();
			
			if (!supportsMove(context, functionType.returnType())) {
				throw ErrorException(makeString("Return type '%s' of function '%s' is not movable, at position %s.",
					functionType.returnType()->toString().c_str(),
					semFunction->name().toString().c_str(),
					astFunctionNode.location().toString().c_str()));
			}
			
			if (astFunctionNode->isDefaultDefinition()) {
				// Has a default definition.
				CreateDefaultMethod(context, lookupParentType(context.scopeStack()), semFunction, astFunctionNode.location());
				return;
			}
			
			if (astFunctionNode->isDeclaration()) {
				// Only a declaration.
				return;
			}
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			auto semScope = ConvertScope(context, astFunctionNode->scope());
			
			const auto returnType = functionType.returnType();
			const auto exitStates = semScope->exitStates();
			
			assert(!exitStates.hasBreakExit());
			assert(!exitStates.hasContinueExit());
			assert(!exitStates.hasRethrowExit());
			
			if (exitStates.hasNormalExit()) {
				if (!returnType->isBuiltInVoid()) {
					// Functions with non-void return types must return a value.
					throw ErrorException(makeString("Control reaches end of function '%s' with non-void return type; it needs a return statement, at location %s.",
						semFunction->name().toString().c_str(),
						astFunctionNode.location().toString().c_str()));
				} else {
					// Need to add a void return statement if the program didn't.
					semScope->statements().push_back(SEM::Statement::ReturnVoid());
				}
			}
			
			DeadCodeSearchScope(context, *semScope);
			
			const auto actualNoexceptPredicate = reducePredicate(context, exitStates.noexceptPredicate().copy());
			const auto declaredNoexceptPredicate = reducePredicate(context, functionType.attributes().noExceptPredicate().copy());
			
			if (!declaredNoexceptPredicate.implies(actualNoexceptPredicate)) {
				if (declaredNoexceptPredicate.isTrue() && actualNoexceptPredicate.isFalse()) {
					throw ErrorException(makeString("Function '%s' is declared as 'noexcept' but can throw, at location %s.",
					                                semFunction->name().toString().c_str(),
					                                astFunctionNode.location().toString().c_str()));
				}
				
				throw ErrorException(makeString("Function '%s' has noexcept predicate '%s' which isn't implied by explicitly declared noexcept predicate '%s', at location %s.",
					semFunction->name().toString().c_str(),
					exitStates.noexceptPredicate().toString().c_str(),
					functionType.attributes().noExceptPredicate().toString().c_str(),
					astFunctionNode.location().toString().c_str()));
			}
			
			semFunction->setScope(std::move(semScope));
			
			// Check all variables are either used and not marked unused,
			// or are unused and marked as such.
			for (const auto& varPair: semFunction->namedVariables()) {
				const auto& varName = varPair.first;
				const auto& var = varPair.second;
				if (var->isUsed() && var->isMarkedUnused()) {
					const auto& debugInfo = var->debugInfo();
					assert(debugInfo);
					const auto& location = debugInfo->declLocation;
					throw ErrorException(makeString("Parameter variable '%s' is marked unused but is used in function '%s', at position %s.",
						varName.c_str(), semFunction->name().toString().c_str(), location.toString().c_str()));
				} else if (!var->isUsed() && !var->isMarkedUnused()) {
					const auto& debugInfo = var->debugInfo();
					assert(debugInfo);
					const auto& location = debugInfo->declLocation;
					throw ErrorException(makeString("Parameter variable '%s' is unused in function '%s', at position %s.",
						varName.c_str(), semFunction->name().toString().c_str(), location.toString().c_str()));
				}
			}
			
		}
		
	}
	
}

