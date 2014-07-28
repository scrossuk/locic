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
		
		llvm::Function* getAssertFailedFunction(Module& module) {
			const std::string functionName = "__loci_assert_failed";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			llvm::Type* const argTypes[] = { typeGen.getI8PtrType() };
			const auto functionType = typeGen.getVoidFunctionType(argTypes);
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
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
						const auto thenBB = basicBlocks[i * 2 + 0];
						const auto elseBB = basicBlocks[i * 2 + 1];
						
						// If this is the last if clause and there's no else clause,
						// have the false condition jump straight to the merge.
						const bool isEnd = ((i + 1) == ifClauseList.size());
						const auto nextBB = (!isEnd || hasElseScope) ? elseBB : mergeBB;
						
						llvm::Value* conditionValue = nullptr;
						bool conditionHasUnwindActions = false;
						bool thenClauseTerminated = false;
						
						{
							ScopeLifetime ifScopeLifetime(function);
							
							conditionValue = genValue(function, ifClause->condition());
							conditionHasUnwindActions = anyUnwindCleanupActions(function, UnwindStateNormal);
							
							// The condition value may involve some unwinding operations, in
							// which case we need to jump to the next unwind block if the
							// condition was false. Once unwinding is complete then we may
							// need to re-check the condition to determine where to proceed.
							if (conditionHasUnwindActions) {
								function.getBuilder().CreateCondBr(conditionValue, thenBB, genUnwindBlock(function, UnwindStateNormal));
							} else {
								function.getBuilder().CreateCondBr(conditionValue, thenBB, nextBB);
							}
							
							// Create 'then'.
							function.selectBasicBlock(thenBB);
							genScope(function, ifClause->scope());
							
							if (lastInstructionTerminates(function)) {
								thenClauseTerminated = true;
								// The 'then' clause finished with a terminator (e.g. a 'return'),
								// but we need a new basic block for the unwinding operations
								// from the if condition (in case the condition was false).
								if (conditionHasUnwindActions) {
									const auto ifUnwindBB = function.createBasicBlock("ifUnwind");
									function.selectBasicBlock(ifUnwindBB);
								}
							} else {
								// The 'then' clause did not end with a terminator, so
								// it will necessary to generate a final merge block.
								allTerminate = false;
							}
						}
						
						if (!lastInstructionTerminates(function)) {
							if (conditionHasUnwindActions) {
								if (thenClauseTerminated) {
									// The 'then' clause terminated, so we can just jump straight
									// to the next clause.
									function.getBuilder().CreateBr(nextBB);
								} else if (nextBB == mergeBB) {
									// The merge block is the next block, so just jump there.
									function.getBuilder().CreateBr(mergeBB);
								} else {
									// Need to discern between success/failure cases.
									function.getBuilder().CreateCondBr(conditionValue, mergeBB, nextBB);
								}
							} else {
								function.getBuilder().CreateBr(mergeBB);
							}
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
					const auto switchValue = genValue(function, statement->getSwitchValue());
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
						condition = genValue(function, statement->getLoopCondition());
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
					if (anyUnwindActions(function, UnwindStateReturn)) {
						if (statement->getReturnValue() != nullptr && !statement->getReturnValue()->type()->isBuiltInVoid()) {
							if (function.getArgInfo().hasReturnVarArgument()) {
								const auto returnValue = genValue(function, statement->getReturnValue());
								
								// Store the return value into the return value pointer.
								genStore(function, returnValue, function.getReturnVar(), statement->getReturnValue()->type());
							} else {
								const auto returnValue = genValue(function, statement->getReturnValue());
								
								const auto encodedReturnValue = encodeReturnValue(function, returnValue,
									genABIType(function.module(), statement->getReturnValue()->type()));
								
								function.setReturnValue(encodedReturnValue);
							}
						}
						
						genUnwind(function, UnwindStateReturn);
					} else {
						llvm::Instruction* returnInst = nullptr;
						
						if (statement->getReturnValue() != nullptr && !statement->getReturnValue()->type()->isBuiltInVoid()) {
							if (function.getArgInfo().hasReturnVarArgument()) {
								const auto returnValue = genValue(function, statement->getReturnValue());
								
								// Store the return value into the return value pointer.
								genStore(function, returnValue, function.getReturnVar(), statement->getReturnValue()->type());
								
								returnInst = function.getBuilder().CreateRetVoid();
							} else {
								const auto returnValue = genValue(function, statement->getReturnValue());
								
								const auto encodedReturnValue = encodeReturnValue(function, returnValue,
									genABIType(function.module(), statement->getReturnValue()->type()));
								
								returnInst = function.getBuilder().CreateRet(encodedReturnValue);
							}
						} else {
							returnInst = function.getBuilder().CreateRetVoid();
						}
						
						if (hasDebugInfo) {
							returnInst->setDebugLoc(debugLocation);
						}
					}
					
					break;
				}
				
				case SEM::Statement::TRY: {
					assert(!statement->getTryCatchList().empty());
					
					// Get list of exception types to be caught by this statement.
					llvm::SmallVector<llvm::Constant*, 5> catchTypeList;
					
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
						const auto castedCatchTypeInfo = ConstantGenerator(module).getPointerCast(catchTypeList[i], TypeGenerator(module).getI8PtrType());
						const auto catchSelectorValue = function.getBuilder().CreateCall(intrinsic, std::vector<llvm::Value*> {castedCatchTypeInfo});
						
						// Check thrown selector against catch selector.
						const auto compareResult = function.getBuilder().CreateICmpEQ(catchSelectorValue, throwSelectorValue);
						function.getBuilder().CreateCondBr(compareResult, executeCatchBB, tryNextCatchBB);
						
						// If matched, execute catch block and then continue normal execution.
						{
							function.selectBasicBlock(executeCatchBB);
							const auto catchType = genType(function.module(), catchClause->var()->constructType());
							llvm::Value* const getPtrArgs[] = { thrownExceptionValue };
							const auto exceptionPtrValue = function.getBuilder().CreateCall(getExceptionPtrFunction(module), getPtrArgs);
							const auto castedExceptionValue = function.getBuilder().CreatePointerCast(exceptionPtrValue, catchType->getPointerTo());
							
							assert(catchClause->var()->isBasic());
							function.getLocalVarMap().forceInsert(catchClause->var(), castedExceptionValue);
							
							{
								ScopeLifetime catchScopeLifetime(function);
								
								// Turn 'rethrow' state into 'throw' state when
								// unwinding out of this scope.
								function.pushUnwindAction(UnwindAction::RethrowScope());
								
								// Make sure the exception object is freed at the end
								// of the catch block (unless it is rethrown, in which
								// case this action won't be run and the action above
								// will turn the state to 'throw').
								function.pushUnwindAction(UnwindAction::DestroyException(exceptionPtrValue));
								
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
					genUnwind(function, UnwindStateThrow);
					
					if (!allTerminate) {
						function.selectBasicBlock(afterCatchBB);
					} else {
						afterCatchBB->eraseFromParent();
					}
					break;
				}
				
				case SEM::Statement::THROW: {
					const auto throwType = statement->getThrowValue()->type();
					
					const auto exceptionValue = genValue(function, statement->getThrowValue());
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
					
					if (anyUnwindActions(function, UnwindStateThrow)) {
						// Create throw and nothrow paths.
						const auto noThrowPath = function.createBasicBlock("");
						const auto throwPath = genLandingPad(function, UnwindStateThrow);
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
					
					// Call 'rethrow' function.
					const auto rethrowFunction = getExceptionRethrowFunction(module);
					llvm::Value* const args[] = { exceptionValue };
					
					// Only generate landing pad where necessary.
					if (anyUnwindRethrowActions(function)) {
						// Create throw and nothrow paths.
						const auto noThrowPath = function.createBasicBlock("");
						const auto throwPath = genLandingPad(function, UnwindStateRethrow);
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
					genUnwind(function, UnwindStateBreak);
					break;
				}
				
				case SEM::Statement::CONTINUE: {
					genUnwind(function, UnwindStateContinue);
					break;
				}
				
				case SEM::Statement::ASSERT: {
					const auto failBB = function.createBasicBlock("assertFail");
					const auto successBB = function.createBasicBlock("assertSuccess");
					
					const auto conditionValue = genValue(function, statement->getAssertValue());
					function.getBuilder().CreateCondBr(conditionValue, successBB, failBB);
					
					function.selectBasicBlock(failBB);
					
					const auto stringValue = statement->getAssertName();
					
					const auto arrayType = TypeGenerator(module).getArrayType(TypeGenerator(module).getI8Type(), stringValue.size() + 1);
					const auto constArray = ConstantGenerator(module).getString(stringValue.c_str());
					const auto globalArray = module.createConstGlobal("assert_name_constant", arrayType, llvm::GlobalValue::PrivateLinkage, constArray);
					globalArray->setAlignment(1);
					const auto stringGlobal = function.getBuilder().CreateConstGEP2_32(globalArray, 0, 0);
					
					const auto assertFailedFunction = getAssertFailedFunction(module);
					llvm::Value* const args[] = { stringGlobal };
					function.getBuilder().CreateCall(assertFailedFunction, args);
					function.getBuilder().CreateUnreachable();
					
					function.selectBasicBlock(successBB);
					break;
				}
				
				default:
					llvm_unreachable("Unknown statement type");
			}
		}
		
	}
	
}

