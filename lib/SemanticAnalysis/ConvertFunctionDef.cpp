#include <cassert>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ExitStates.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void DeadCodeSearchScope(Context& context, SEM::Scope* scope);
		
		void DeadCodeSearchStatement(Context& context, SEM::Statement* statement) {
			switch(statement->kind()) {
				case SEM::Statement::VALUE:
				case SEM::Statement::INITIALISE:
				case SEM::Statement::RETURN:
				case SEM::Statement::THROW:
				case SEM::Statement::RETHROW:
				case SEM::Statement::BREAK:
				case SEM::Statement::CONTINUE:
				case SEM::Statement::ASSERT:
				case SEM::Statement::UNREACHABLE: {
					return;
				}
				case SEM::Statement::SCOPE: {
					DeadCodeSearchScope(context, &(statement->getScope()));
					return;
				}
				case SEM::Statement::IF: {
					for (const auto ifClause: statement->getIfClauseList()) {
						DeadCodeSearchScope(context, &(ifClause->scope()));
					}
					
					DeadCodeSearchScope(context, &(statement->getIfElseScope()));
					return;
				}
				case SEM::Statement::SWITCH: {
					for (auto switchCase: statement->getSwitchCaseList()) {
						DeadCodeSearchScope(context, &(switchCase->scope()));
					}
					return;
				}
				case SEM::Statement::LOOP: {
					DeadCodeSearchScope(context, &(statement->getLoopIterationScope()));
					DeadCodeSearchScope(context, &(statement->getLoopAdvanceScope()));
					return;
				}
				case SEM::Statement::TRY: {
					DeadCodeSearchScope(context, &(statement->getTryScope()));
					for (auto catchClause: statement->getTryCatchList()) {
						DeadCodeSearchScope(context, &(catchClause->scope()));
					}
					return;
				}
				case SEM::Statement::SCOPEEXIT: {
					DeadCodeSearchScope(context, &(statement->getScopeExitScope()));
					return;
				}
				default: {
					throw std::runtime_error("Unknown statement kind.");
				}
			}
		}
		
		void DeadCodeSearchScope(Context& context, SEM::Scope* scope) {
			bool isNormalBlocked = false;
			for (const auto statement: scope->statements()) {
				DeadCodeSearchStatement(context, statement);
				
				if (isNormalBlocked) {
					auto& debugModule = context.debugModule();
					const auto iterator = debugModule.statementMap.find(statement);
					assert(iterator != debugModule.statementMap.end());
					throw ErrorException(makeString("Function contains dead code beginning with statement at position %s.",
						iterator->second.location.toString().c_str()));
				}
				
				const auto exitStates = GetStatementExitStates(statement);
				if (!exitStates.test(UnwindStateNormal)) {
					isNormalBlocked = true;
				}
			}
		}
		
		void ConvertFunctionDef(Context& context, const AST::Node<AST::Function>& astFunctionNode) {
			const auto semFunction = context.scopeStack().back().function();
			
			// Function should currently be a declaration
			// (it is about to be made into a definition).
			assert(semFunction->isDeclaration());
			
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
			const auto semScope = ConvertScope(context, astFunctionNode->scope());
			
			const auto returnType = semFunction->type()->getFunctionReturnType();
			
			const auto exitStates = GetScopeExitStates(semScope);
			
			assert(!exitStates.test(UnwindStateBreak));
			assert(!exitStates.test(UnwindStateContinue));
			assert(!exitStates.test(UnwindStateRethrow));
			
			if (exitStates.test(UnwindStateNormal)) {
				if (!returnType->isBuiltInVoid()) {
					// Functions with non-void return types must return a value.
					throw MissingReturnStatementException(semFunction->name());
				} else {
					// Need to add a void return statement if the program didn't.
					semScope->statements().push_back(SEM::Statement::ReturnVoid());
				}
			}
			
			DeadCodeSearchScope(context, semScope);
			
			if (semFunction->type()->isFunctionNoExcept() && exitStates.test(UnwindStateThrow)) {
				throw ErrorException(makeString("Function '%s' is declared as 'noexcept' but can throw, at location %s.",
					semFunction->name().toString().c_str(),
					astFunctionNode.location().toString().c_str()));
			}
			
			semFunction->setScope(semScope);
		}
		
	}
	
}

