#include <assert.h>

#include <llvm/Value.h>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/ConstantGenerator.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenStatement.hpp>
#include <Locic/CodeGen/GenValue.hpp>
#include <Locic/CodeGen/Memory.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		void genScope(Function& function, const SEM::Scope& scope) {
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
			switch (statement->kind()) {
				case SEM::Statement::VALUE: {
					genValue(function, statement->getValue());
					break;
				}
				
				case SEM::Statement::SCOPE: {
					genScope(function, statement->getScope());
					break;
				}
				
				case SEM::Statement::IF: {
					llvm::BasicBlock* thenBB = function.createBasicBlock("then");
					llvm::BasicBlock* elseBB = function.createBasicBlock("else");
					llvm::BasicBlock* mergeBB = function.createBasicBlock("ifmerge");
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
					llvm::BasicBlock* insideLoopBB = function.createBasicBlock("insideLoop");
					llvm::BasicBlock* afterLoopBB = function.createBasicBlock("afterLoop");
					function.getBuilder().CreateCondBr(genValue(function, statement->getWhileCondition()),
													   insideLoopBB, afterLoopBB);
													   
					// Create loop contents.
					function.selectBasicBlock(insideLoopBB);
					genScope(function, statement->getWhileScope());
					function.getBuilder().CreateCondBr(genValue(function, statement->getWhileCondition()),
													   insideLoopBB, afterLoopBB);
													   
					// Create after loop basic block (which is where execution continues).
					function.selectBasicBlock(afterLoopBB);
					break;
				}
				
				case SEM::Statement::ASSIGN: {
					SEM::Value* lValue = statement->getAssignLValue();
					SEM::Value* rValue = statement->getAssignRValue();
					genStore(function, genValue(function, rValue), genValue(function, lValue, true), rValue->type());
					break;
				}
				
				case SEM::Statement::RETURN: {
					if (statement->getReturnValue() != NULL
						&& !statement->getReturnValue()->type()->isVoid()) {
						llvm::Value* returnValue = genValue(function, statement->getReturnValue());
						
						if (function.getArgInfo().hasReturnVarArgument()) {
							genStore(function, returnValue, function.getReturnVar(), statement->getReturnValue()->type());
							function.getBuilder().CreateRetVoid();
						} else {
							function.getBuilder().CreateRet(returnValue);
						}
					} else {
						function.getBuilder().CreateRetVoid();
					}
					
					// Need a basic block after a return statement in case anything more is generated.
					// This (and any following code) will be removed by dead code elimination.
					function.selectBasicBlock(function.createBasicBlock("next"));
					break;
				}
				
				default:
					assert(false && "Unknown statement type");
					break;
			}
		}
		
	}
	
}

