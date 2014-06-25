#include <assert.h>

#include <stdexcept>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/ControlFlow.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genScope(Function& function, const SEM::Scope& scope) {
			ScopeLifetime scopeLifetime(function);
			
			for (const auto localVar: scope.variables()) {
				genVarAlloca(function, localVar);
			}
			
			for (const auto statement: scope.statements()) {
				genStatement(function, statement);
			}
		}
		
		llvm::Value* genStatementValue(Function& function, SEM::Value* value) {
			// Ensure any objects that only exist until the end of
			// the statement are destroyed.
			StatementLifetime statementLifetime(function);
			return genValue(function, value);
		}
		
		void genStatement(Function& function, SEM::Statement* statement) {
			auto& module = function.module();
			auto& statementMap = module.debugModule().statementMap;
			const auto iterator = statementMap.find(statement);
			const auto hasDebugInfo = (iterator != statementMap.end());
			const auto debugSourceLocation = hasDebugInfo ? iterator->second.location : Debug::SourceLocation::Null();
			const auto debugStartPosition = debugSourceLocation.range().start();
			const auto debugLocation = llvm::DebugLoc::get(debugStartPosition.lineNumber(), debugStartPosition.column(), function.debugInfo());
			
			switch (statement->kind()) {
				case SEM::Statement::VALUE: {
					assert(statement->getValue()->type()->isBuiltInVoid());
					(void) genStatementValue(function, statement->getValue());
					break;
				}
				
				case SEM::Statement::SCOPE: {
					genScope(function, statement->getScope());
					break;
				}
				
				case SEM::Statement::INITIALISE: {
					const auto var = statement->getInitialiseVar();
					const auto value = genStatementValue(function, statement->getInitialiseValue());
					genVarInitialise(function, var, value);
					break;
				}
				
				case SEM::Statement::IF: {
					const auto mergeBB = function.createBasicBlock("ifMerge");
					
					assert(!statement->getIfClauseList().empty());
					
					for (const auto ifClause: statement->getIfClauseList()) {
						const auto conditionValue = genStatementValue(function, ifClause->condition());
						
						const auto thenBB = function.createBasicBlock("ifThen");
						const auto elseBB = function.createBasicBlock("ifElse");
						
						function.getBuilder().CreateCondBr(conditionValue, thenBB, elseBB);
						
						// Create 'then'.
						function.selectBasicBlock(thenBB);
						genScope(function, ifClause->scope());
						function.getBuilder().CreateBr(mergeBB);
						
						// Create 'else'.
						function.selectBasicBlock(elseBB);
					}
					
					genScope(function, statement->getIfElseScope());
					
					function.getBuilder().CreateBr(mergeBB);
					
					// Create merge (which is where execution continues).
					function.selectBasicBlock(mergeBB);
					break;
				}
				
				case SEM::Statement::SWITCH: {
					const auto switchValue = genStatementValue(function, statement->getSwitchValue());
					const auto switchType = statement->getSwitchValue()->type();
					
					llvm::Value* switchValuePtr = nullptr;
					
					if (!switchValue->getType()->isPointerTy()) {
						switchValuePtr = genAlloca(function, statement->getSwitchValue()->type());
						genStore(function, switchValue, switchValuePtr, statement->getSwitchValue()->type());
					} else {
						switchValuePtr = switchValue;
					}
					
					const auto loadedTagPtr = function.getBuilder().CreateConstInBoundsGEP2_32(switchValuePtr, 0, 0);
					const auto loadedTag = function.getBuilder().CreateLoad(loadedTagPtr);
					
					const auto unionValuePtr = function.getBuilder().CreateConstInBoundsGEP2_32(switchValuePtr, 0, 1);
					
					const auto endBB = function.createBasicBlock("switchEnd");
					const auto switchInstruction = function.getBuilder().CreateSwitch(loadedTag, endBB, statement->getSwitchCaseList().size());
					
					for (auto switchCase: statement->getSwitchCaseList()) {
						const auto caseType = switchCase->var()->constructType();
						uint8_t tag = 0;
						for (auto variantTypeInstance: switchType->getObjectType()->variants()) {
							if (variantTypeInstance == caseType->getObjectType()) break;
							tag++;
						}
						
						const auto tagValue = ConstantGenerator(function.module()).getI8(tag);
						const auto caseBB = function.createBasicBlock("switchCase");
						
						switchInstruction->addCase(tagValue, caseBB);
						
						function.selectBasicBlock(caseBB);
						
						const auto unionValueType = genType(function.module(), caseType);
						const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionValuePtr, unionValueType->getPointerTo());
						
						{
							ScopeLifetime switchCaseLifetime(function);
							genVarAlloca(function, switchCase->var());
							genVarInitialise(function, switchCase->var(), genLoad(function, castedUnionValuePtr, switchCase->var()->constructType()));
							genScope(function, switchCase->scope());
						}
						
						function.getBuilder().CreateBr(endBB);
					}
					
					function.selectBasicBlock(endBB);
					break;
				}
				
				case SEM::Statement::LOOP: {
					const auto loopConditionBB = function.createBasicBlock("loopCondition");
					const auto loopIterationBB = function.createBasicBlock("loopIteration");
					const auto loopAdvanceBB = function.createBasicBlock("loopAdvance");
					const auto loopEndBB = function.createBasicBlock("loopEnd");
					
					// Execution starts in the condition block.
					function.getBuilder().CreateBr(loopConditionBB);
					function.selectBasicBlock(loopConditionBB);
					
					llvm::Value* condition = nullptr;
					
					// Ensure destructors for conditional expression are generated
					// before the branch instruction.
					{
						ScopeLifetime conditionScopeLifetime(function);
						condition = genStatementValue(function, statement->getLoopCondition());
					}
					
					function.getBuilder().CreateCondBr(condition, loopIterationBB, loopEndBB);
					
					// Create loop contents.
					function.selectBasicBlock(loopIterationBB);
					
					{
						ControlFlowScope controlFlowScope(function, loopEndBB, loopAdvanceBB);
						genScope(function, statement->getLoopIterationScope());
					}
					
					// At the end of a loop iteration, branch to
					// the advance block to update any data for
					// the next iteration.
					function.getBuilder().CreateBr(loopAdvanceBB);
					
					function.selectBasicBlock(loopAdvanceBB);
					
					genScope(function, statement->getLoopAdvanceScope());
					
					// Now branch back to the start to re-check the condition.
					function.getBuilder().CreateBr(loopConditionBB);
													   
					// Create after loop basic block (which is where execution continues).
					function.selectBasicBlock(loopEndBB);
					break;
				}
				
				case SEM::Statement::RETURN: {
					if (statement->getReturnValue() != nullptr && !statement->getReturnValue()->type()->isBuiltInVoid()) {
						if (function.getArgInfo().hasReturnVarArgument()) {
							const auto returnValue = genStatementValue(function, statement->getReturnValue());
							
							// Store the return value into the return value pointer.
							genStore(function, returnValue, function.getReturnVar(), statement->getReturnValue()->type());
						} else {
							// Return the value directly, which actually involves it
							// being stored in a stack allocated block and then loaded
							// and returned in the top unwind block.
							// TODO: add debug location.
							function.setReturnValue(genStatementValue(function, statement->getReturnValue()));
						}
					}
					
					// Set unwind state to 'return'.
					setCurrentUnwindState(function, UnwindStateReturn);
					
					// Jump to next unwind block, which will eventually lead
					// to a return statement in the top unwind block.
					function.getBuilder().CreateBr(getNextUnwindBlock(function));
					
					// Basic block for any further instructions generated.
					function.selectBasicBlock(function.createBasicBlock("afterReturn"));
					break;
				}
				
				case SEM::Statement::TRY: {
					assert(!statement->getTryCatchList().empty());
					
					// Get list of exception types to be caught by this statement.
					std::vector<llvm::Constant*> catchTypeList;
					for (const auto catchClause: statement->getTryCatchList()) {
						catchTypeList.push_back(genCatchInfo(module, catchClause->var()->constructType()->getObjectType()));
					}
					
					assert(catchTypeList.size() == statement->getTryCatchList().size());
					
					const auto catchBB = function.createBasicBlock("catch");
					const auto afterCatchBB = function.createBasicBlock("afterCatch");
					
					// Execute the 'try' scope, pushing the exception
					// handlers onto the unwind stack.
					{
						TryScope tryScope(function.unwindStack(), catchBB, catchTypeList);
						genScope(function, statement->getTryScope());
					}
					
					// No exception thrown; continue normal execution.
					function.getBuilder().CreateBr(afterCatchBB);
					
					function.selectBasicBlock(catchBB);
					
					const auto nextUnwindBB = getNextUnwindBlock(function);
					
					// Check if unwinding due an exception.
					const auto isExceptionState = getIsCurrentExceptState(function);
					
					const auto handleExceptBB = function.createBasicBlock("");
					function.getBuilder().CreateCondBr(isExceptionState, handleExceptBB, nextUnwindBB);
					function.selectBasicBlock(handleExceptBB);
					
					// Load selector of exception thrown.
					const auto exceptionInfo = function.getBuilder().CreateLoad(function.exceptionInfo());
					const auto thrownExceptionValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned>{0});
					const auto throwSelectorValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned>{1});
					
					for (size_t i = 0; i < statement->getTryCatchList().size(); i++) {
						const auto catchClause = statement->getTryCatchList().at(i);
						const auto executeCatchBB = function.createBasicBlock("executeCatch");
						const auto tryNextCatchBB = function.createBasicBlock("tryNextCatch");
						
						// Call llvm.eh.typeid.for intrinsic to get
						// the selector for the catch type.
						const auto intrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::eh_typeid_for, std::vector<llvm::Type*>{});
						const auto castedCatchTypeInfo = ConstantGenerator(module).getPointerCast(catchTypeList.at(i), TypeGenerator(module).getI8PtrType());
						const auto catchSelectorValue = function.getBuilder().CreateCall(intrinsic, std::vector<llvm::Value*>{castedCatchTypeInfo});
						
						// Check thrown selector against catch selector.
						const auto compareResult = function.getBuilder().CreateICmpEQ(catchSelectorValue, throwSelectorValue);
						function.getBuilder().CreateCondBr(compareResult, executeCatchBB, tryNextCatchBB);
						
						// If matched, execute catch block and then continue normal execution.
						{
							function.selectBasicBlock(executeCatchBB);
							const auto catchType = genType(function.module(), catchClause->var()->constructType());
							const auto exceptionPtrValue = function.getBuilder().CreateCall(getExceptionPtrFunction(module),
								std::vector<llvm::Value*>{ thrownExceptionValue });
							const auto castedExceptionValue = function.getBuilder().CreatePointerCast(exceptionPtrValue, catchType->getPointerTo());
							
							assert(catchClause->var()->isBasic());
							function.getLocalVarMap().forceInsert(catchClause->var(), castedExceptionValue);
							
							// Set unwind state to 'normal'.
							setCurrentUnwindState(function, UnwindStateNormal);
							
							{
								ScopeLifetime catchScopeLifetime(function);
								
								// Make sure the exception object is freed at the end
								// of the catch block (unless it is rethrown).
								scheduleExceptionDestroy(function, exceptionPtrValue);
								
								genScope(function, catchClause->scope());
							}
							
							// Exception was handled, so re-commence normal execution.
							function.getBuilder().CreateBr(afterCatchBB);
						}
						
						function.selectBasicBlock(tryNextCatchBB);
					}
					
					// If not matched, keep unwinding.
					function.getBuilder().CreateBr(nextUnwindBB);
					
					function.selectBasicBlock(afterCatchBB);
					break;
				}
				
				case SEM::Statement::THROW: {
					auto throwType = statement->getThrowValue()->type();
					
					const auto exceptionValue = genStatementValue(function, statement->getThrowValue());
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
					const auto throwInvoke = function.getBuilder().CreateInvoke(throwFunction, noThrowPath, throwPath,
						std::vector<llvm::Value*>{ allocatedException, castedTypeInfo, nullPtr });
					
					if (hasDebugInfo) {
						throwInvoke->setDebugLoc(debugLocation);
					}
					
					// ==== 'throw' function doesn't throw: Should never happen.
					function.selectBasicBlock(noThrowPath);
					function.getBuilder().CreateUnreachable();
					
					// ==== 'throw' function DOES throw: Landing pad for running destructors/catch blocks.
					function.selectBasicBlock(throwPath);
					
					const bool isRethrow = false;
					genLandingPad(function, isRethrow);
					
					// Basic block for any further instructions generated.
					function.selectBasicBlock(function.createBasicBlock("afterThrow"));
					break;
				}
				
				case SEM::Statement::RETHROW: {
					llvm::Value* exceptionValue = nullptr;
					
					const auto& unwindStack = function.unwindStack();
					for (size_t i = 0; i < unwindStack.size(); i++) {
						const auto pos = unwindStack.size() - i - 1;
						const auto& unwindElement = unwindStack.at(pos);
						
						if (unwindElement.isCatchBlock()) {
							exceptionValue = unwindElement.catchExceptionValue();
							break;
						}
					}
					
					assert(exceptionValue != nullptr);
					
					const auto noThrowPath = function.createBasicBlock("throwFail");
					const auto throwPath = function.createBasicBlock("throwLandingPad");
					
					// Call 'rethrow' function.
					const auto rethrowFunction = getExceptionRethrowFunction(module);
					
					const auto rethrowInvoke = function.getBuilder().CreateInvoke(rethrowFunction, noThrowPath, throwPath,
						std::vector<llvm::Value*>{ exceptionValue });
					
					if (hasDebugInfo) {
						rethrowInvoke->setDebugLoc(debugLocation);
					}
					
					// ==== 'rethrow' function doesn't throw: Should never happen.
					function.selectBasicBlock(noThrowPath);
					function.getBuilder().CreateUnreachable();
					
					// ==== 'rethrow' function DOES throw: Landing pad for running destructors/catch blocks.
					function.selectBasicBlock(throwPath);
					
					const bool isRethrow = true;
					genLandingPad(function, isRethrow);
					
					// Basic block for any further instructions generated.
					function.selectBasicBlock(function.createBasicBlock("afterRethrow"));
					break;
				}
				
				case SEM::Statement::SCOPEEXIT: {
					const auto currentBB = function.getSelectedBasicBlock();
					
					if (statement->getScopeExitState() == "exit") {
						// scope(exit) is run for all unwind states.
						
						const auto scopeExitBB = function.createBasicBlock("scopeExit");
						function.selectBasicBlock(scopeExitBB);
						genScope(function, statement->getScopeExitScope());
						function.getBuilder().CreateBr(getNextUnwindBlock(function));
						
						function.unwindStack().push_back(UnwindAction::ScopeExit(scopeExitBB));
					} else if (statement->getScopeExitState() == "success") {
						// scope(success) is only run when an exception is NOT active.
						
						const auto scopeSuccessBB = function.createBasicBlock("scopeSuccess");
						function.selectBasicBlock(scopeSuccessBB);
						
						const auto runActionBB = function.createBasicBlock("");
						const auto afterActionBB = getNextUnwindBlock(function);
						
						const auto isExceptState = getIsCurrentExceptState(function);
						function.getBuilder().CreateCondBr(isExceptState, afterActionBB, runActionBB);
						
						function.selectBasicBlock(runActionBB);
						genScope(function, statement->getScopeExitScope());
						function.getBuilder().CreateBr(afterActionBB);
						
						function.unwindStack().push_back(UnwindAction::ScopeExit(scopeSuccessBB));
					} else if (statement->getScopeExitState() == "failure") {
						// scope(failure) is only run when an exception is active.
						
						const auto scopeFailureBB = function.createBasicBlock("scopeFailure");
						function.selectBasicBlock(scopeFailureBB);
						
						const auto runActionBB = function.createBasicBlock("");
						const auto afterActionBB = getNextUnwindBlock(function);
						
						const auto isExceptState = getIsCurrentExceptState(function);
						function.getBuilder().CreateCondBr(isExceptState, runActionBB, afterActionBB);
						
						function.selectBasicBlock(runActionBB);
						genScope(function, statement->getScopeExitScope());
						function.getBuilder().CreateBr(afterActionBB);
						
						function.unwindStack().push_back(UnwindAction::ScopeExit(scopeFailureBB));
					} else {
						llvm_unreachable("Unknown scope exit action kind.");
					}
					
					function.selectBasicBlock(currentBB);
					break;
				}
				
				case SEM::Statement::BREAK: {
					genControlFlowBreak(function);
					
					// Basic block for any further instructions generated.
					function.selectBasicBlock(function.createBasicBlock("afterThrow"));
					break;
				}
				
				case SEM::Statement::CONTINUE: {
					genControlFlowContinue(function);
					
					// Basic block for any further instructions generated.
					function.selectBasicBlock(function.createBasicBlock("afterThrow"));
					break;
				}
				
				default:
					llvm_unreachable("Unknown statement type");
			}
		}
		
	}
	
}

