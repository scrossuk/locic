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
			
			for (const auto localVar : scope.variables()) {
				genVarAlloca(function, localVar);
			}
			
			for (const auto statement : scope.statements()) {
				genStatement(function, statement);
			}
		}
		
		static llvm::Value* encodeReturnValue(Function& function, llvm::Value* value, llvm_abi::Type* type) {
			std::vector<llvm::Value*> values;
			values.push_back(value);
			
			llvm_abi::Type* const abiTypes[] = { type };
			function.module().abi().encodeValues(function.getEntryBuilder(), function.getBuilder(), values, abiTypes);
			
			return values.at(0);
		}
		
		llvm::Value* genStatementValue(Function& function, SEM::Value* value) {
			// Ensure any objects that only exist until the end of
			// the statement are destroyed.
			StatementLifetime statementLifetime(function);
			return genValue(function, value);
		}
		
		static bool lastInstructionTerminates(Function& function) {
			if (!function.getBuilder().GetInsertBlock()->empty()) {
				auto iterator = function.getBuilder().GetInsertPoint();
				--iterator;
				return iterator->isTerminator();
			} else {
				return false;
			}
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
					const auto& ifClauseList = statement->getIfClauseList();
					assert(!ifClauseList.empty());
					
					// Create basic blocks in program order.
					llvm::SmallVector<llvm::BasicBlock*, 5> basicBlocks;
					for (size_t i = 0; i < ifClauseList.size(); i++) {
						basicBlocks.push_back(function.createBasicBlock("ifThen"));
						basicBlocks.push_back(function.createBasicBlock("ifElse"));
					}
					const auto mergeBB = function.createBasicBlock("ifMerge");
					
					const auto& elseScope = statement->getIfElseScope();
					const bool hasElseScope = !elseScope.statements().empty();
					
					bool allTerminate = true;
					
					// Go through all if clauses and generate their code.
					for (size_t i = 0; i < ifClauseList.size(); i++) {
						const auto& ifClause = ifClauseList.at(i);
						const auto conditionValue = genStatementValue(function, ifClause->condition());
						
						const auto thenBB = basicBlocks[i * 2 + 0];
						const auto elseBB = basicBlocks[i * 2 + 1];
						
						// If this is the last if clause and there's no else clause,
						// have the false condition jump straight to the merge.
						const bool isEnd = ((i + 1) == ifClauseList.size());
						function.getBuilder().CreateCondBr(conditionValue, thenBB, (!isEnd || hasElseScope) ? elseBB : mergeBB);
						
						// Create 'then'.
						function.selectBasicBlock(thenBB);
						genScope(function, ifClause->scope());
						
						if (!lastInstructionTerminates(function)) {
							allTerminate = false;
							function.getBuilder().CreateBr(mergeBB);
						}
						
						// Create 'else'.
						function.selectBasicBlock(elseBB);
					}
					
					// Only generate the else basic block if there is
					// an else scope, otherwise erase it.
					if (hasElseScope) {
						genScope(function, statement->getIfElseScope());
						
						if (!lastInstructionTerminates(function)) {
							allTerminate = false;
							function.getBuilder().CreateBr(mergeBB);
						}
					} else {
						allTerminate = false;
						basicBlocks.back()->eraseFromParent();
						basicBlocks.pop_back();
						function.selectBasicBlock(basicBlocks.back());
					}
					
					if (allTerminate) {
						// If every if clause terminates, then erase
						// the merge block and terminate here.
						mergeBB->eraseFromParent();
					} else {
						// Select merge block (which is where execution continues).
						function.selectBasicBlock(mergeBB);
					}
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
					
					// TODO: implement default case.
					const auto defaultBB = function.createBasicBlock("");
					
					const auto switchInstruction = function.getBuilder().CreateSwitch(loadedTag, defaultBB, statement->getSwitchCaseList().size());
					
					bool allTerminate = true;
					
					for (auto switchCase : statement->getSwitchCaseList()) {
						const auto caseType = switchCase->var()->constructType();
						uint8_t tag = 0;
						
						for (auto variantTypeInstance : switchType->getObjectType()->variants()) {
							if (variantTypeInstance == caseType->getObjectType()) {
								break;
							}
							
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
						
						if (!lastInstructionTerminates(function)) {
							allTerminate = false;
							function.getBuilder().CreateBr(endBB);
						}
					}
					
					function.selectBasicBlock(defaultBB);
					function.getBuilder().CreateUnreachable();
					
					if (allTerminate) {
						endBB->eraseFromParent();
					} else {
						function.selectBasicBlock(endBB);
					}
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
					llvm::Instruction* returnInst = nullptr;
					
					if (statement->getReturnValue() != nullptr && !statement->getReturnValue()->type()->isBuiltInVoid()) {
						if (function.getArgInfo().hasReturnVarArgument()) {
							const auto returnValue = genStatementValue(function, statement->getReturnValue());
							
							// Store the return value into the return value pointer.
							genStore(function, returnValue, function.getReturnVar(), statement->getReturnValue()->type());
							
							// Perform all exit actions.
							genAllScopeExitActions(function);
							
							returnInst = function.getBuilder().CreateRetVoid();
						} else {
							const auto returnValue = genStatementValue(function, statement->getReturnValue());
							
							const auto encodedReturnValue = encodeReturnValue(function, returnValue, genABIType(function.module(), statement->getReturnValue()->type()));
							
							// Perform all exit actions.
							genAllScopeExitActions(function);
							
							returnInst = function.getBuilder().CreateRet(encodedReturnValue);
						}
					} else {
						// Perform all exit actions.
						genAllScopeExitActions(function);
						
						returnInst = function.getBuilder().CreateRetVoid();
					}
					
					if (hasDebugInfo) {
						returnInst->setDebugLoc(debugLocation);
					}
					
					break;
				}
				
				case SEM::Statement::TRY: {
					assert(!statement->getTryCatchList().empty());
					
					// Get list of exception types to be caught by this statement.
					std::vector<llvm::Constant*> catchTypeList;
					
					for (const auto catchClause : statement->getTryCatchList()) {
						catchTypeList.push_back(genCatchInfo(module, catchClause->var()->constructType()->getObjectType()));
					}
					
					assert(catchTypeList.size() == statement->getTryCatchList().size());
					
					const auto catchBB = function.createBasicBlock("catch");
					const auto afterCatchBB = function.createBasicBlock("");
					
					// Execute the 'try' scope, pushing the exception
					// handlers onto the unwind stack.
					{
						TryScope tryScope(function, catchBB, catchTypeList);
						genScope(function, statement->getTryScope());
					}
					
					bool allTerminate = true;
					
					if (!lastInstructionTerminates(function)) {
						// No exception thrown; continue normal execution.
						allTerminate = false;
						function.getBuilder().CreateBr(afterCatchBB);
					}
					
					function.selectBasicBlock(catchBB);
					
					// Load selector of exception thrown.
					const auto exceptionInfo = function.getBuilder().CreateLoad(function.exceptionInfo());
					const auto thrownExceptionValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned> {0});
					const auto throwSelectorValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned> {1});
					
					for (size_t i = 0; i < statement->getTryCatchList().size(); i++) {
						const auto catchClause = statement->getTryCatchList().at(i);
						const auto executeCatchBB = function.createBasicBlock("executeCatch");
						const auto tryNextCatchBB = function.createBasicBlock("tryNextCatch");
						
						// Call llvm.eh.typeid.for intrinsic to get
						// the selector for the catch type.
						const auto intrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::eh_typeid_for, std::vector<llvm::Type*> {});
						const auto castedCatchTypeInfo = ConstantGenerator(module).getPointerCast(catchTypeList.at(i), TypeGenerator(module).getI8PtrType());
						const auto catchSelectorValue = function.getBuilder().CreateCall(intrinsic, std::vector<llvm::Value*> {castedCatchTypeInfo});
						
						// Check thrown selector against catch selector.
						const auto compareResult = function.getBuilder().CreateICmpEQ(catchSelectorValue, throwSelectorValue);
						function.getBuilder().CreateCondBr(compareResult, executeCatchBB, tryNextCatchBB);
						
						// If matched, execute catch block and then continue normal execution.
						{
							function.selectBasicBlock(executeCatchBB);
							const auto catchType = genType(function.module(), catchClause->var()->constructType());
							const auto exceptionPtrValue = function.getBuilder().CreateCall(getExceptionPtrFunction(module),
														   std::vector<llvm::Value*> { thrownExceptionValue });
							const auto castedExceptionValue = function.getBuilder().CreatePointerCast(exceptionPtrValue, catchType->getPointerTo());
							
							assert(catchClause->var()->isBasic());
							function.getLocalVarMap().forceInsert(catchClause->var(), castedExceptionValue);
							
							{
								ScopeLifetime catchScopeLifetime(function);
								
								// Make sure the exception object is freed at the end
								// of the catch block (unless it is rethrown).
								scheduleExceptionDestroy(function, exceptionPtrValue);
								
								genScope(function, catchClause->scope());
							}
							
							// Exception was handled, so re-commence normal execution.
							if (!lastInstructionTerminates(function)) {
								allTerminate = false;
								function.getBuilder().CreateBr(afterCatchBB);
							}
						}
						
						function.selectBasicBlock(tryNextCatchBB);
					}
					
					// If not matched, keep unwinding.
					const bool isRethrow = false;
					genExceptionUnwind(function, exceptionInfo, isRethrow);
					
					if (!allTerminate) {
						function.selectBasicBlock(afterCatchBB);
					} else {
						afterCatchBB->eraseFromParent();
					}
					break;
				}
				
				case SEM::Statement::THROW: {
					const auto throwType = statement->getThrowValue()->type();
					
					const auto exceptionValue = genStatementValue(function, statement->getThrowValue());
					const auto exceptionType = genType(module, throwType);
					
					// Allocate space for exception.
					const auto allocateFunction = getExceptionAllocateFunction(module);
					const auto exceptionValueSize = genSizeOf(function, throwType);
					llvm::Value* const exceptArgs[] = { exceptionValueSize };
					const auto allocatedException = function.getBuilder().CreateCall(allocateFunction, exceptArgs);
					
					// Store value into allocated space.
					const auto castedAllocatedException = function.getBuilder().CreatePointerCast(allocatedException, exceptionType->getPointerTo());
					genStore(function, exceptionValue, castedAllocatedException, throwType);
					
					// Call 'throw' function.
					const auto throwFunction = getExceptionThrowFunction(module);
					const auto throwTypeInfo = genThrowInfo(module, throwType->getObjectType());
					const auto castedTypeInfo = function.getBuilder().CreatePointerCast(throwTypeInfo, TypeGenerator(module).getI8PtrType());
					const auto nullPtr = ConstantGenerator(module).getNull(TypeGenerator(module).getI8PtrType());
					llvm::Value* const args[] = { allocatedException, castedTypeInfo, nullPtr };
					
					const bool isRethrow = false;
					
					if (anyExceptionActions(function, isRethrow)) {
						// Create throw and nothrow paths.
						const auto noThrowPath = function.createBasicBlock("");
						const auto throwPath = genLandingPad(function, isRethrow);
						const auto throwInvoke = function.getBuilder().CreateInvoke(throwFunction, noThrowPath, throwPath, args);
						
						if (hasDebugInfo) {
							throwInvoke->setDebugLoc(debugLocation);
						}
						
						// 'throw' function should never return normally.
						function.selectBasicBlock(noThrowPath);
						function.getBuilder().CreateUnreachable();
					} else {
						const auto callInst = function.getBuilder().CreateCall(throwFunction, args);
						
						if (hasDebugInfo) {
							callInst->setDebugLoc(debugLocation);
						}
						
						// 'throw' function should never return normally.
						function.getBuilder().CreateUnreachable();
					}
					break;
				}
				
				case SEM::Statement::RETHROW: {
					llvm::Value* exceptionValue = nullptr;
					
					const auto& unwindStack = function.unwindStack();
					
					for (size_t i = 0; i < unwindStack.size(); i++) {
						const auto pos = unwindStack.size() - i - 1;
						const auto& unwindElement = unwindStack.at(pos);
						
						if (unwindElement.isDestroyException()) {
							exceptionValue = unwindElement.destroyExceptionValue();
							break;
						}
					}
					
					assert(exceptionValue != nullptr);
					
					// Call 'throw' function.
					const auto rethrowFunction = getExceptionRethrowFunction(module);
					llvm::Value* const args[] = { exceptionValue };
					
					const bool isRethrow = true;
					
					// Only generate landing pad where necessary.
					if (anyExceptionActions(function, isRethrow)) {
						// Create throw and nothrow paths.
						const auto noThrowPath = function.createBasicBlock("");
						const auto throwPath = genLandingPad(function, isRethrow);
						const auto throwInvoke = function.getBuilder().CreateInvoke(rethrowFunction, noThrowPath, throwPath, args);
						
						if (hasDebugInfo) {
							throwInvoke->setDebugLoc(debugLocation);
						}
						
						// 'rethrow' function should never return normally.
						function.selectBasicBlock(noThrowPath);
						function.getBuilder().CreateUnreachable();
					} else {
						const auto callInst = function.getBuilder().CreateCall(rethrowFunction, args);
						
						if (hasDebugInfo) {
							callInst->setDebugLoc(debugLocation);
						}
						
						// 'rethrow' function should never return normally.
						function.getBuilder().CreateUnreachable();
					}
					break;
				}
				
				case SEM::Statement::SCOPEEXIT: {
					ScopeExitState state = SCOPEEXIT_ALWAYS;
					
					if (statement->getScopeExitState() == "exit") {
						state = SCOPEEXIT_ALWAYS;
					} else if (statement->getScopeExitState() == "success") {
						state = SCOPEEXIT_SUCCESS;
					} else if (statement->getScopeExitState() == "failure") {
						state = SCOPEEXIT_FAILURE;
					}
					
					function.pushUnwindAction(UnwindAction::ScopeExit(state, &(statement->getScopeExitScope())));
					break;
				}
				
				case SEM::Statement::BREAK: {
					genControlFlowBreak(function);
					break;
				}
				
				case SEM::Statement::CONTINUE: {
					genControlFlowContinue(function);
					break;
				}
				
				default:
					llvm_unreachable("Unknown statement type");
			}
		}
		
	}
	
}

