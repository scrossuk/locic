#include <assert.h>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genScope(Function& function, const SEM::Scope& scope) {
			LifetimeScope lifetimeScope(function);
			
			for (std::size_t i = 0; i < scope.localVariables().size(); i++) {
				const auto localVar = scope.localVariables().at(i);
				
				// Create an alloca for this variable.
				llvm::Value* stackObject = genAlloca(function, localVar->type());
				
				function.getLocalVarMap().forceInsert(localVar, stackObject);
			}
			
			for (std::size_t i = 0; i < scope.statements().size(); i++) {
				genStatement(function, scope.statements().at(i));
			}
		}
		
		void genStatement(Function& function, SEM::Statement* statement) {
			switch (statement->kind()) {
				case SEM::Statement::VALUE: {
					assert(statement->getValue()->type()->isVoid());
					(void) genValue(function, statement->getValue());
					break;
				}
				
				case SEM::Statement::SCOPE: {
					genScope(function, statement->getScope());
					break;
				}
				
				case SEM::Statement::INITIALISE: {
					const auto varValue = function.getLocalVarMap().get(statement->getInitialiseVar());
					const auto initialiseValue = genValue(function, statement->getInitialiseValue());
					genStoreVar(function, initialiseValue, varValue, statement->getInitialiseValue()->type(), statement->getInitialiseVar()->type());
					
					// Add this to the list of variables to be
					// destroyed at the end of the function.
					assert(!function.destructorScopeStack().empty());
					function.destructorScopeStack().back().push_back(std::make_pair(statement->getInitialiseVar()->type(), varValue));
					break;
				}
				
				case SEM::Statement::IF: {
					const auto conditionBB = function.createBasicBlock("ifCondition");
					const auto thenBB = function.createBasicBlock("ifThen");
					const auto elseBB = function.createBasicBlock("ifElse");
					const auto mergeBB = function.createBasicBlock("ifMerge");
					
					function.getBuilder().CreateBr(conditionBB);
					function.selectBasicBlock(conditionBB);
					
					function.getBuilder().CreateCondBr(genValue(function, statement->getIfCondition()),
													   thenBB, elseBB);
													   
					// Create 'then'.
					function.selectBasicBlock(thenBB);
					genScope(function, statement->getIfTrueScope());
					function.getBuilder().CreateBr(mergeBB);
					
					// Create 'else'.
					function.selectBasicBlock(elseBB);
					
					if (statement->hasIfFalseScope()) {
						genScope(function, statement->getIfFalseScope());
					}
					
					function.getBuilder().CreateBr(mergeBB);
					
					// Create merge (which is where execution continues).
					function.selectBasicBlock(mergeBB);
					break;
				}
				
				case SEM::Statement::WHILE: {
					const auto conditionBB = function.createBasicBlock("whileConditionLoop");
					const auto insideLoopBB = function.createBasicBlock("whileInsideLoop");
					const auto afterLoopBB = function.createBasicBlock("whileAfterLoop");
					
					// Execution starts in the condition block.
					function.getBuilder().CreateBr(conditionBB);
					function.selectBasicBlock(conditionBB);
					
					llvm::Value* condition = NULL;
					
					// Ensure destructors for conditional expression are generated
					// before the branch instruction.
					{
						LifetimeScope conditionLifetimeScope(function);
						condition = genValue(function, statement->getWhileCondition());
					}
					
					function.getBuilder().CreateCondBr(condition, insideLoopBB, afterLoopBB);
													   
					// Create loop contents.
					function.selectBasicBlock(insideLoopBB);
					genScope(function, statement->getWhileScope());
					
					// At the end of a loop iteration, branch back
					// to the start to re-check the condition.
					function.getBuilder().CreateBr(conditionBB);
													   
					// Create after loop basic block (which is where execution continues).
					function.selectBasicBlock(afterLoopBB);
					break;
				}
				
				case SEM::Statement::RETURN: {
					if (statement->getReturnValue() != NULL
						&& !statement->getReturnValue()->type()->isVoid()) {
						if (function.getArgInfo().hasReturnVarArgument()) {
							const auto returnValue = genValue(function, statement->getReturnValue());
							
							// Store the return value into the return value pointer.
							genStore(function, returnValue, function.getReturnVar(), statement->getReturnValue()->type());
							
							// Call all destructors.
							genAllScopeDestructorCalls(function);
							
							function.getBuilder().CreateRetVoid();
						} else {
							const auto returnValue = genValue(function, statement->getReturnValue());
							
							// Call all destructors.
							genAllScopeDestructorCalls(function);
							
							function.getBuilder().CreateRet(returnValue);
						}
					} else {
						// Call all destructors.
						genAllScopeDestructorCalls(function);
						
						function.getBuilder().CreateRetVoid();
					}
					
					// Need a basic block after a return statement in case anything more is generated.
					// This (and any following code) will be removed by dead code elimination.
					function.selectBasicBlock(function.createBasicBlock("afterReturn"));
					break;
				}
				
				default:
					assert(false && "Unknown statement type");
					break;
			}
		}
		
	}
	
}

