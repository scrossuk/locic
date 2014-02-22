#include <assert.h>

#include <stdexcept>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genVar(Function& function, SEM::Var* var) {
			if (var->isAny()) return;
			
			if (var->isBasic()) {
				// Create an alloca for this variable.
				const auto stackObject = genAlloca(function, var->type());
				function.getLocalVarMap().forceInsert(var, stackObject);
			} else if (var->isComposite()) {
				// Generate child vars.
				for (const auto childVar: var->children()) {
					genVar(function, childVar);
				}
			} else {
				throw std::runtime_error("Unknown var kind.");
			}
		}
		
		void genScope(Function& function, const SEM::Scope& scope) {
			LifetimeScope lifetimeScope(function);
			
			for (std::size_t i = 0; i < scope.localVariables().size(); i++) {
				const auto localVar = scope.localVariables().at(i);
				genVar(function, localVar);
			}
			
			for (std::size_t i = 0; i < scope.statements().size(); i++) {
				genStatement(function, scope.statements().at(i));
			}
		}
		
		void genVarInitialise(Function& function, SEM::Var* var, llvm::Value* initialiseValue) {
			if (var->isAny()) {
				// Casting to 'any', which means the destructor
				// should be called for the value.
				genDestructorCall(function, var->constructType(), initialiseValue);
			} else if (var->isBasic()) {
				const auto varValue = function.getLocalVarMap().get(var);
				genStoreVar(function, initialiseValue, varValue, var);
				
				// Add this to the list of variables to be
				// destroyed at the end of the function.
				function.unwindStack().push_back(UnwindAction::Destroy(var->type(), varValue));
			} else if (var->isComposite()) {
				// For composite variables, extract each member of
				// the type and assign it to its variable.
				for (size_t i = 0; i < var->children().size(); i++) {
					const auto childVar = var->children().at(i);
					const auto childInitialiseValue = function.getBuilder().CreateConstInBoundsGEP2_32(initialiseValue, 0, i);
					const auto loadedChildInitialiseValue = genLoad(function, childInitialiseValue, childVar->constructType());
					genVarInitialise(function, childVar, loadedChildInitialiseValue);
				}
			} else {
				throw std::runtime_error("Unknown var kind.");
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
					const auto var = statement->getInitialiseVar();
					const auto value = genValue(function, statement->getInitialiseValue());
					genVarInitialise(function, var, value);
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
				
				case SEM::Statement::SWITCH: {
					const auto switchValue = genValue(function, statement->getSwitchValue());
					const auto switchType = statement->getSwitchValue()->type();
					
					const auto loadedTagPtr = function.getBuilder().CreateConstInBoundsGEP2_32(switchValue, 0, 0);
					const auto loadedTag = function.getBuilder().CreateLoad(loadedTagPtr);
					
					const auto unionValuePtr = function.getBuilder().CreateConstInBoundsGEP2_32(switchValue, 0, 1);
					
					const auto endBB = function.createBasicBlock("switchEnd");
					const auto switchInstruction = function.getBuilder().CreateSwitch(loadedTag, endBB, statement->getSwitchCaseList().size());
					
					for (auto switchCase: statement->getSwitchCaseList()) {
						auto caseType = switchCase->var()->constructType();
						uint8_t tag = 0;
						for (auto variantTypeInstance: switchType->getObjectType()->variants()) {
							if (variantTypeInstance == caseType->getObjectType()) break;
							tag++;
						}
						
						const auto tagValue = ConstantGenerator(function.getModule()).getI8(tag);
						const auto caseBB = function.createBasicBlock("switchCase");
						
						switchInstruction->addCase(tagValue, caseBB);
						
						function.selectBasicBlock(caseBB);
						
						const auto unionValueType = genType(function.getModule(), caseType);
						const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionValuePtr, unionValueType->getPointerTo());
						
						{
							LifetimeScope lifetimeScope(function);
							genVar(function, switchCase->var());
							genVarInitialise(function, switchCase->var(), castedUnionValuePtr);
							genScope(function, switchCase->scope());
						}
						
						function.getBuilder().CreateBr(endBB);
					}
					
					function.selectBasicBlock(endBB);
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
				
				case SEM::Statement::TRY: {
					auto& module = function.getModule();
					
					// TODO: add support for multiple catch clauses.
					assert(statement->getTryCatchList().size() == 1 && "Multiple catch clauses currently not supported.");
					
					const auto catchClause = statement->getTryCatchList().front();
					const auto catchTypeInfo = genCatchInfo(module, catchClause->var()->constructType()->getObjectType());
					
					const auto catchBlock = function.createBasicBlock("catch");
					{
						TryScope tryScope(function.unwindStack(), catchBlock, catchTypeInfo);
						genScope(function, statement->getTryScope());
					}
					
					const auto afterCatchBlock = function.createBasicBlock("afterCatch");
					function.getBuilder().CreateBr(afterCatchBlock);
					
					function.selectBasicBlock(catchBlock);
					
					// Call llvm.eh.typeid.for intrinsic to get
					// the selector for the catch type.
					const auto intrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::eh_typeid_for, std::vector<llvm::Type*>{});
					const auto castedCatchTypeInfo = ConstantGenerator(module).getPointerCast(catchTypeInfo, TypeGenerator(module).getI8PtrType());
					const auto catchSelectorValue = function.getBuilder().CreateCall(intrinsic, std::vector<llvm::Value*>{castedCatchTypeInfo});
					
					// Load selector of exception thrown.
					const auto exceptionInfo = function.getBuilder().CreateLoad(function.exceptionInfo());
					const auto thrownExceptionValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned>{0});
					const auto throwSelectorValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned>{1});
					
					const auto doCatchBlock = function.createBasicBlock("doCatch");
					const auto continueUnwindBlock = function.createBasicBlock("continueUnwind");
					
					// Check thrown selector against catch selector.
					const auto compareResult = function.getBuilder().CreateICmpEQ(catchSelectorValue, throwSelectorValue);
					function.getBuilder().CreateCondBr(compareResult, doCatchBlock, continueUnwindBlock);
					
					// If matched, run catch block and then continue normal execution.
					function.selectBasicBlock(doCatchBlock);
					{
						const auto catchType = genType(function.getModule(), catchClause->var()->constructType());
						const auto exceptionDataValue = function.getBuilder().CreateCall(getBeginCatchFunction(module), std::vector<llvm::Value*>{thrownExceptionValue});
						const auto castedExceptionValue = function.getBuilder().CreatePointerCast(exceptionDataValue, catchType->getPointerTo());
						
						genVar(function, catchClause->var());
						genVarInitialise(function, catchClause->var(), castedExceptionValue);
						genScope(function, catchClause->scope());
						
						// TODO: make sure this gets called if an exception if thrown in the handler!
						function.getBuilder().CreateCall(getEndCatchFunction(module), std::vector<llvm::Value*>{});
						
						function.getBuilder().CreateBr(afterCatchBlock);
					}
					
					// If not matched, keep unwinding.
					function.selectBasicBlock(continueUnwindBlock);
					genExceptionUnwind(function);
					
					function.selectBasicBlock(afterCatchBlock);
					break;
				}
				
				case SEM::Statement::THROW: {
					auto& module = function.getModule();
					auto throwType = statement->getThrowValue()->type();
					
					const auto exceptionValue = genValue(function, statement->getThrowValue());
					const auto exceptionType = genType(module, throwType);
					
					// Allocate space for exception.
					const auto allocateFunction = getExceptionAllocateFunction(module);
					const auto exceptionValueSize = genSizeOf(function, throwType);
					const auto allocatedException = function.getBuilder().CreateCall(allocateFunction, std::vector<llvm::Value*>{ exceptionValueSize });
					
					// Store value into allocated space.
					const auto castedAllocatedException = function.getBuilder().CreatePointerCast(allocatedException, exceptionType->getPointerTo());
					genStore(function, exceptionValue, castedAllocatedException, throwType);
					
					const auto noThrowPath = function.createBasicBlock("throwFail");
					const auto throwPath = function.createBasicBlock("throwLandingPad");
					
					// Call 'throw' function.
					const auto throwFunction = getExceptionThrowFunction(module);
					const auto throwTypeInfo = genThrowInfo(module, throwType->getObjectType());
					const auto castedTypeInfo = function.getBuilder().CreatePointerCast(throwTypeInfo, TypeGenerator(module).getI8PtrType());
					const auto nullPtr = ConstantGenerator(module).getNull(TypeGenerator(module).getI8PtrType());
					function.getBuilder().CreateInvoke(throwFunction, noThrowPath, throwPath, std::vector<llvm::Value*>{ allocatedException, castedTypeInfo, nullPtr });
					
					// ==== 'throw' function doesn't throw: Should never happen.
					function.selectBasicBlock(noThrowPath);
					function.getBuilder().CreateUnreachable();
					
					// ==== 'throw' function DOES throw: Landing pad for running destructors/catch blocks.
					function.selectBasicBlock(throwPath);
					genLandingPad(function);
					
					// Basic block for any further instructions generated.
					function.selectBasicBlock(function.createBasicBlock("afterThrow"));
					break;
				}
				
				default:
					assert(false && "Unknown statement type");
					break;
			}
		}
		
	}
	
}

