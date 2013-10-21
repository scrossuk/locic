#include <assert.h>

#include <llvm/Value.h>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/ConstantGenerator.hpp>
#include <Locic/CodeGen/Destructor.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenStatement.hpp>
#include <Locic/CodeGen/GenValue.hpp>
#include <Locic/CodeGen/Memory.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		void genScope(Function& function, const SEM::Scope& scope) {
			LifetimeScope lifetimeScope(function);
			
			for (std::size_t i = 0; i < scope.localVariables().size(); i++) {
				SEM::Var* localVar = scope.localVariables().at(i);
				
				// Create an alloca for this variable.
				llvm::Value* stackObject = genAlloca(function, localVar->type());
				
				function.getLocalVarMap().forceInsert(localVar, stackObject);
			}
			
			for (std::size_t i = 0; i < scope.statements().size(); i++) {
				genStatement(function, scope.statements().at(i));
			}
		}
		
		void genStatement(Function& function, SEM::Statement* statement) {
			LifetimeScope lifetimeScope(function);
			
			switch (statement->kind()) {
				case SEM::Statement::VALUE: {
					genValue(function, statement->getValue());
					break;
				}
				
				case SEM::Statement::SCOPE: {
					genScope(function, statement->getScope());
					break;
				}
				
				case SEM::Statement::INITIALISE: {
					llvm::Value* varValue = function.getLocalVarMap().get(statement->getInitialiseVar());
					genStore(function, genValue(function, statement->getInitialiseValue()), varValue, statement->getInitialiseValue()->type());
					break;
				}
				
				case SEM::Statement::IF: {
					llvm::BasicBlock* conditionBB = function.createBasicBlock("ifCondition");
					llvm::BasicBlock* thenBB = function.createBasicBlock("ifThen");
					llvm::BasicBlock* elseBB = function.createBasicBlock("ifElse");
					llvm::BasicBlock* mergeBB = function.createBasicBlock("ifMerge");
					
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
					llvm::BasicBlock* conditionBB = function.createBasicBlock("whileConditionLoop");
					llvm::BasicBlock* insideLoopBB = function.createBasicBlock("whileInsideLoop");
					llvm::BasicBlock* afterLoopBB = function.createBasicBlock("whileAfterLoop");
					
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
							llvm::Value* returnValue = genValue(function, statement->getReturnValue());
							
							// Store the return value into the return value pointer.
							genStore(function, returnValue, function.getReturnVar(), statement->getReturnValue()->type());
							
							// Call all destructors.
							genAllScopeDestructorCalls(function);
							
							function.getBuilder().CreateRetVoid();
						} else {
							llvm::Value* returnValue = genValue(function, statement->getReturnValue());
							
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

