#include <assert.h>

#include <stdexcept>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/AST/Scope.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/ControlFlow.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/ScopeEmitter.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/StatementEmitter.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/ValueEmitter.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ArgInfo assertFailedArgInfo(Module& module) {
			const auto voidType = std::make_pair(llvm_abi::VoidTy, TypeGenerator(module).getVoidType());
			const auto voidPtr = std::make_pair(llvm_abi::PointerTy, TypeGenerator(module).getPtrType());
			
			const TypePair argTypes[] = { voidPtr };
			return ArgInfo::Basic(module, voidType, argTypes).withNoExcept().withNoReturn();
		}
		
		llvm::Function* getAssertFailedFunction(Module& module) {
			const auto functionName = module.getCString("__loci_assert_failed");
			const auto iterator = module.getFunctionMap().find(functionName);
			
			if (iterator != module.getFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto function = createLLVMFunction(module, assertFailedArgInfo(module), llvm::Function::ExternalLinkage, functionName);
			module.getFunctionMap().insert(std::make_pair(functionName, function));
			return function;
		}
		
		ArgInfo unreachableFailedArgInfo(Module& module) {
			const auto voidType = std::make_pair(llvm_abi::VoidTy, TypeGenerator(module).getVoidType());
			return ArgInfo::Basic(module, voidType, {}).withNoExcept().withNoReturn();
		}
		
		llvm::Function* getUnreachableFailedFunction(Module& module) {
			const auto functionName = module.getCString("__loci_unreachable_failed");
			const auto iterator = module.getFunctionMap().find(functionName);
			
			if (iterator != module.getFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto function = createLLVMFunction(module, unreachableFailedArgInfo(module), llvm::Function::ExternalLinkage, functionName);
			module.getFunctionMap().insert(std::make_pair(functionName, function));
			return function;
		}
		
		StatementEmitter::StatementEmitter(IREmitter& irEmitter)
		: irEmitter_(irEmitter) { }
		
		void StatementEmitter::emitStatement(const SEM::Statement& statement) {
			const auto& debugInfo = statement.debugInfo();
			if (debugInfo) {
				irEmitter_.function().setDebugPosition(
				    debugInfo->location.range().start());
			}
			
			switch (statement.kind()) {
				case SEM::Statement::VALUE: {
					emitValue(statement.getValue());
					return;
				}
				case SEM::Statement::SCOPE: {
					emitScope(statement.getScope());
					return;
				}
				case SEM::Statement::INITIALISE: {
					emitInitialise(statement.getInitialiseVar(),
					               statement.getInitialiseValue());
					return;
				}
				case SEM::Statement::IF: {
					emitIf(statement.getIfClauseList(),
					       statement.getIfElseScope());
					return;
				}
				case SEM::Statement::SWITCH: {
					emitSwitch(statement.getSwitchValue(),
					           statement.getSwitchCaseList(),
					           statement.getSwitchDefaultScope());
					return;
				}
				case SEM::Statement::LOOP: {
					emitLoop(statement.getLoopCondition(),
					         statement.getLoopIterationScope(),
					         statement.getLoopAdvanceScope());
					return;
				}
				case SEM::Statement::FOR: {
					emitFor(statement.getForVar(),
					        statement.getForInitValue(),
					        statement.getForScope());
					return;
				}
				case SEM::Statement::RETURNVOID: {
					emitReturnVoid();
					return;
				}
				case SEM::Statement::RETURN: {
					emitReturn(statement.getReturnValue());
					return;
				}
				case SEM::Statement::TRY: {
					emitTry(statement.getTryScope(),
					        statement.getTryCatchList());
					return;
				}
				case SEM::Statement::THROW: {
					emitThrow(statement.getThrowValue());
					return;
				}
				case SEM::Statement::RETHROW: {
					emitRethrow();
					return;
				}
				case SEM::Statement::SCOPEEXIT: {
					emitScopeExit(statement.getScopeExitState(),
					              statement.getScopeExitScope());
					return;
				}
				case SEM::Statement::BREAK: {
					emitBreak();
					return;
				}
				case SEM::Statement::CONTINUE: {
					emitContinue();
					return;
				}
				case SEM::Statement::ASSERT: {
					emitAssert(statement.getAssertValue(),
					           statement.getAssertName());
					return;
				}
				
				case SEM::Statement::ASSERTNOEXCEPT: {
					emitAssertNoExcept(statement.getAssertNoExceptScope());
					return;
				}
				
				case SEM::Statement::UNREACHABLE: {
					emitUnreachable();
					return;
				}
			}
			
			llvm_unreachable("Unknown statement type");
		}
		
		void StatementEmitter::emitValue(const AST::Value& value) {
			assert(value.type()->isBuiltInVoid());
			ValueEmitter valueEmitter(irEmitter_);
			(void) valueEmitter.emitValue(value);
		}
		
		void StatementEmitter::emitScope(const AST::Scope& scope) {
			ScopeEmitter(irEmitter_).emitScope(scope);
		}
		
		void StatementEmitter::emitInitialise(AST::Var& var,
		                                      const AST::Value& value) {
			const auto varAllocaOptional = irEmitter_.function().getLocalVarMap().tryGet(&var);
			const auto varAlloca = varAllocaOptional ? *varAllocaOptional : nullptr;
			
			ValueEmitter valueEmitter(irEmitter_);
			const auto valueIR = valueEmitter.emitValue(value, varAlloca);
			genVarInitialise(irEmitter_.function(), &var, valueIR);
		}
		
		void StatementEmitter::emitIf(const std::vector<SEM::IfClause*>& ifClauseList,
		                              const AST::Scope& elseScope) {
			assert(!ifClauseList.empty());
			
			auto& function = irEmitter_.function();
			ValueEmitter valueEmitter(irEmitter_);
			
			// Create basic blocks in program order.
			llvm::SmallVector<llvm::BasicBlock*, 5> basicBlocks;
			for (size_t i = 0; i < ifClauseList.size(); i++) {
				basicBlocks.push_back(irEmitter_.createBasicBlock("ifThen"));
				basicBlocks.push_back(irEmitter_.createBasicBlock("ifElse"));
			}
			const auto mergeBB = irEmitter_.createBasicBlock("ifMerge");
			
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
					
					const auto boolCondition = valueEmitter.emitValue(ifClause->condition());
					conditionValue = irEmitter_.emitBoolToI1(boolCondition);
					conditionHasUnwindActions = anyUnwindCleanupActions(function, UnwindStateNormal);
					
					// The condition value may involve some unwinding operations, in
					// which case we need to jump to the next unwind block if the
					// condition was false. Once unwinding is complete then we may
					// need to re-check the condition to determine where to proceed.
					if (conditionHasUnwindActions) {
						irEmitter_.emitCondBranch(conditionValue, thenBB, genUnwindBlock(function, UnwindStateNormal));
					} else {
						irEmitter_.emitCondBranch(conditionValue, thenBB, nextBB);
					}
					
					// Create 'then'.
					irEmitter_.selectBasicBlock(thenBB);
					ScopeEmitter(irEmitter_).emitScope(ifClause->scope());
					
					if (irEmitter_.lastInstructionTerminates()) {
						thenClauseTerminated = true;
						// The 'then' clause finished with a terminator (e.g. a 'return'),
						// but we need a new basic block for the unwinding operations
						// from the if condition (in case the condition was false).
						if (conditionHasUnwindActions) {
							const auto ifUnwindBB = irEmitter_.createBasicBlock("ifUnwind");
							irEmitter_.selectBasicBlock(ifUnwindBB);
						}
					} else {
						// The 'then' clause did not end with a terminator, so
						// it will necessary to generate a final merge block.
						allTerminate = false;
					}
				}
				
				if (!irEmitter_.lastInstructionTerminates()) {
					if (conditionHasUnwindActions) {
						if (thenClauseTerminated) {
							// The 'then' clause terminated, so we can just jump straight
							// to the next clause.
							irEmitter_.emitBranch(nextBB);
						} else if (nextBB == mergeBB) {
							// The merge block is the next block, so just jump there.
							irEmitter_.emitBranch(mergeBB);
						} else {
							// Need to discern between success/failure cases.
							irEmitter_.emitCondBranch(conditionValue, mergeBB, nextBB);
						}
					} else {
						irEmitter_.emitBranch(mergeBB);
					}
				}
				
				// Create 'else'.
				irEmitter_.selectBasicBlock(elseBB);
			}
			
			// Only generate the else basic block if there is
			// an else scope, otherwise erase it.
			if (hasElseScope) {
				ScopeEmitter(irEmitter_).emitScope(elseScope);
				
				if (!irEmitter_.lastInstructionTerminates()) {
					allTerminate = false;
					irEmitter_.emitBranch(mergeBB);
				}
			} else {
				allTerminate = false;
				basicBlocks.back()->eraseFromParent();
				basicBlocks.pop_back();
				irEmitter_.selectBasicBlock(basicBlocks.back());
			}
			
			if (allTerminate) {
				// If every if clause terminates, then erase
				// the merge block and terminate here.
				mergeBB->eraseFromParent();
			} else {
				// Select merge block (which is where execution continues).
				irEmitter_.selectBasicBlock(mergeBB);
			}
		}
		
		void StatementEmitter::emitSwitch(const AST::Value& switchValue,
		                                  const std::vector<SEM::SwitchCase*>& switchCases,
		                                  const AST::Scope* defaultScope) {
			assert(switchValue.type()->isUnionDatatype() ||
			       (switchValue.type()->isRef() &&
			        switchValue.type()->isBuiltInReference()));
			
			auto& function = irEmitter_.function();
			auto& module = irEmitter_.module();
			ValueEmitter valueEmitter(irEmitter_);
			
			const bool isSwitchValueRef = switchValue.type()->isRef();
			const auto switchType = isSwitchValueRef ? switchValue.type()->refTarget() : switchValue.type();
			assert(switchType->isUnionDatatype());
			
			const auto llvmSwitchValue = valueEmitter.emitValue(switchValue);
			
			llvm::Value* switchValuePtr = nullptr;
			
			if (isSwitchValueRef) {
				switchValuePtr = llvmSwitchValue;
			} else {
				switchValuePtr = irEmitter_.emitAlloca(switchType);
				irEmitter_.emitMoveStore(llvmSwitchValue, switchValuePtr, switchType);
			}
			
			const auto unionDatatypePointers = getUnionDatatypePointers(function, switchType, switchValuePtr);
			
			const auto loadedTag = irEmitter_.emitRawLoad(unionDatatypePointers.first,
			                                              TypeGenerator(module).getI8Type());
			
			const auto defaultBB = irEmitter_.createBasicBlock("");
			const auto endBB = irEmitter_.createBasicBlock("switchEnd");
			
			const auto switchInstruction = function.getBuilder().CreateSwitch(loadedTag, defaultBB,
			                                                                  switchCases.size());
			
			bool allTerminate = true;
			
			for (auto switchCase: switchCases) {
				const auto caseType = switchCase->var().constructType();
				
				// Start from 1 so 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (auto variantTypeInstance : switchType->getObjectType()->variants()) {
					if (variantTypeInstance == caseType->getObjectType()) {
						break;
					}
					
					tag++;
				}
				
				const auto tagValue = ConstantGenerator(function.module()).getI8(tag);
				const auto caseBB = irEmitter_.createBasicBlock("switchCase");
				
				switchInstruction->addCase(tagValue, caseBB);
				
				irEmitter_.selectBasicBlock(caseBB);
				
				{
					ScopeLifetime switchCaseLifetime(function);
					genVarAlloca(function, &(switchCase->var()));
					genVarInitialise(function, &(switchCase->var()),
						irEmitter_.emitMoveLoad(unionDatatypePointers.second,
						                        switchCase->var().constructType()));
					ScopeEmitter(irEmitter_).emitScope(switchCase->scope());
				}
				
				if (!irEmitter_.lastInstructionTerminates()) {
					allTerminate = false;
					irEmitter_.emitBranch(endBB);
				}
			}
			
			irEmitter_.selectBasicBlock(defaultBB);
			
			if (defaultScope != nullptr) {
				ScopeEmitter(irEmitter_).emitScope(*defaultScope);
				
				if (!irEmitter_.lastInstructionTerminates()) {
					allTerminate = false;
					irEmitter_.emitBranch(endBB);
				}
			} else {
				irEmitter_.emitUnreachable();
			}
			
			if (allTerminate) {
				endBB->eraseFromParent();
			} else {
				irEmitter_.selectBasicBlock(endBB);
			}
		}
		
		void StatementEmitter::emitLoop(const AST::Value& condition,
		                                const AST::Scope& iterationScope,
		                                const AST::Scope& advanceScope) {
			auto& function = irEmitter_.function();
			ValueEmitter valueEmitter(irEmitter_);
			
			const auto loopConditionBB = irEmitter_.createBasicBlock("loopCondition");
			const auto loopIterationBB = irEmitter_.createBasicBlock("loopIteration");
			const auto loopAdvanceBB = irEmitter_.createBasicBlock("loopAdvance");
			const auto loopEndBB = irEmitter_.createBasicBlock("loopEnd");
			
			// Execution starts in the condition block.
			irEmitter_.emitBranch(loopConditionBB);
			irEmitter_.selectBasicBlock(loopConditionBB);
			
			llvm::Value* conditionIR = nullptr;
			
			// Ensure destructors for conditional expression are generated
			// before the branch instruction.
			{
				ScopeLifetime conditionScopeLifetime(function);
				const auto boolCondition = valueEmitter.emitValue(condition);
				conditionIR = irEmitter_.emitBoolToI1(boolCondition);
			}
			
			irEmitter_.emitCondBranch(conditionIR, loopIterationBB,
			                          loopEndBB);
			
			// Create loop contents.
			irEmitter_.selectBasicBlock(loopIterationBB);
			
			{
				ControlFlowScope controlFlowScope(function, loopEndBB, loopAdvanceBB);
				ScopeEmitter(irEmitter_).emitScope(iterationScope);
			}
			
			// At the end of a loop iteration, branch to
			// the advance block to update any data for
			// the next iteration.
			if (!irEmitter_.lastInstructionTerminates()) {
				irEmitter_.emitBranch(loopAdvanceBB);
			}
			
			irEmitter_.selectBasicBlock(loopAdvanceBB);
			
			ScopeEmitter(irEmitter_).emitScope(advanceScope);
			
			// Now branch back to the start to re-check the condition.
			if (!irEmitter_.lastInstructionTerminates()) {
				irEmitter_.emitBranch(loopConditionBB);
			}
			
			// Create after loop basic block (which is where execution continues).
			irEmitter_.selectBasicBlock(loopEndBB);
		}
		
		void StatementEmitter::emitFor(AST::Var& var,
		                               const AST::Value& initValue,
		                               const AST::Scope& scope) {
			/**
			 * This code converts:
			 * for (type value_var: initValue) {
			 *     [for scope]
			 * }
			 * 
			 * ...to (roughly):
			 * 
			 * {
			 *     var iterator = [initValue];
			 *     forCondition:
			 *         if iterator.empty():
			 *             goto forEnd
			 *         else:
			 *             goto forIteration
			 *     forIteration:
			 *     {
			 *         var value_var = iterator.front();
			 *         [for scope]
			 *         goto forAdvance
			 *     }
			 *     forAdvance:
			 *         iterator.skip_front();
			 *         goto forCondition
			 *     forEnd:
			 * }
			 */
			const auto valueType = var.lvalType();
			const auto iteratorType = initValue.type();
			
			auto& function = irEmitter_.function();
			ValueEmitter valueEmitter(irEmitter_);
			
			ScopeLifetime forScopeLifetime(function);
			
			// Create a variable for the iterator/range object.
			const auto iteratorVar = irEmitter_.emitAlloca(iteratorType);
			const auto initValueIR = valueEmitter.emitValue(initValue,
			                                                iteratorVar);
			irEmitter_.emitMoveStore(initValueIR, iteratorVar, iteratorType);
			scheduleDestructorCall(function, iteratorType, iteratorVar);
			
			const auto forConditionBB = irEmitter_.createBasicBlock("forCondition");
			const auto forIterationBB = irEmitter_.createBasicBlock("forIteration");
			const auto forAdvanceBB = irEmitter_.createBasicBlock("forAdvance");
			const auto forEndBB = irEmitter_.createBasicBlock("forEnd");
			
			// Execution starts in the condition block.
			irEmitter_.emitBranch(forConditionBB);
			irEmitter_.selectBasicBlock(forConditionBB);
			
			const auto isEmptyBool = irEmitter_.emitIsEmptyCall(iteratorVar,
			                                                    iteratorType);
			const auto isEmptyI1 = irEmitter_.emitBoolToI1(isEmptyBool);
			
			irEmitter_.emitCondBranch(isEmptyI1, forEndBB,
			                          forIterationBB);
			
			// Create loop contents.
			irEmitter_.selectBasicBlock(forIterationBB);
			
			{
				ScopeLifetime valueScope(function);
				
				// Initialise the loop value.
				const auto varAllocaOptional = function.getLocalVarMap().tryGet(&var);
				const auto varAlloca = varAllocaOptional ? *varAllocaOptional : nullptr;
				const auto value = irEmitter_.emitFrontCall(iteratorVar, iteratorType,
				                                            valueType, varAlloca);
				genVarInitialise(function, &var, value);
				
				ControlFlowScope controlFlowScope(function, forEndBB, forAdvanceBB);
				ScopeEmitter(irEmitter_).emitScope(scope);
			}
			
			// At the end of a loop iteration, branch to
			// the advance block to update any data for
			// the next iteration.
			if (!irEmitter_.lastInstructionTerminates()) {
				irEmitter_.emitBranch(forAdvanceBB);
			}
			
			irEmitter_.selectBasicBlock(forAdvanceBB);
			
			irEmitter_.emitSkipFrontCall(iteratorVar, iteratorType);
			
			// Now branch back to the start to re-check the condition.
			if (!irEmitter_.lastInstructionTerminates()) {
				irEmitter_.emitBranch(forConditionBB);
			}
			
			// Create after loop basic block (which is where execution continues).
			irEmitter_.selectBasicBlock(forEndBB);
		}
		
		void StatementEmitter::emitReturnVoid() {
			auto& function = irEmitter_.function();
			if (anyUnwindActions(function, UnwindStateReturn)) {
				genUnwind(function, UnwindStateReturn);
			} else {
				irEmitter_.emitReturnVoid();
			}
		}
		
		void StatementEmitter::emitReturn(const AST::Value& value) {
			auto& function = irEmitter_.function();
			ValueEmitter valueEmitter(irEmitter_);
			
			if (anyUnwindActions(function, UnwindStateReturn)) {
				if (!value.type()->isBuiltInVoid()) {
					if (function.getArgInfo().hasReturnVarArgument()) {
						const auto returnValue = valueEmitter.emitValue(value,
						                                                function.getReturnVar());
						
						// Store the return value into the return value pointer.
						irEmitter_.emitMoveStore(returnValue, function.getReturnVar(), value.type());
					} else {
						const auto returnValue = valueEmitter.emitValue(value);
						
						// Set the return value to be returned directly later
						// (after executing unwind actions).
						function.setReturnValue(returnValue);
					}
				}
				
				genUnwind(function, UnwindStateReturn);
			} else {
				if (!value.type()->isBuiltInVoid()) {
					if (function.getArgInfo().hasReturnVarArgument()) {
						const auto returnValue = valueEmitter.emitValue(value,
						                                                function.getReturnVar());
						
						// Store the return value into the return value pointer.
						irEmitter_.emitMoveStore(returnValue, function.getReturnVar(), value.type());
						
						irEmitter_.emitReturnVoid();
					} else {
						const auto returnValue = valueEmitter.emitValue(value);
						function.returnValue(returnValue);
					}
				} else {
					irEmitter_.emitReturnVoid();
				}
			}
		}
		
		void StatementEmitter::emitTry(const AST::Scope& scope,
		                               const std::vector<SEM::CatchClause*>& catchClauses) {
			assert(!catchClauses.empty());
			
			auto& function = irEmitter_.function();
			auto& module = irEmitter_.module();
			ValueEmitter valueEmitter(irEmitter_);
			
			// Get list of exception types to be caught by this statement.
			llvm::SmallVector<llvm::Constant*, 5> catchTypeList;
			
			for (const auto catchClause: catchClauses) {
				catchTypeList.push_back(genCatchInfo(module, catchClause->var().constructType()->getObjectType()));
			}
			
			assert(catchTypeList.size() == catchClauses.size());
			
			const auto catchBB = irEmitter_.createBasicBlock("catch");
			const auto afterCatchBB = irEmitter_.createBasicBlock("");
			
			// Execute the 'try' scope, pushing the exception
			// handlers onto the unwind stack.
			{
				TryScope tryScope(function, catchBB, catchTypeList);
				ScopeEmitter(irEmitter_).emitScope(scope);
			}
			
			bool allTerminate = true;
			
			if (!irEmitter_.lastInstructionTerminates()) {
				// No exception thrown; continue normal execution.
				allTerminate = false;
				irEmitter_.emitBranch(afterCatchBB);
			}
			
			irEmitter_.selectBasicBlock(catchBB);
			
			// Load selector of exception thrown.
			TypeGenerator typeGen(module);
			const auto exceptionInfoType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getPtrType(), typeGen.getI32Type()});
			const auto exceptionInfo = irEmitter_.emitRawLoad(function.exceptionInfo(),
			                                                  exceptionInfoType);
			const auto thrownExceptionValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned> {0});
			const auto throwSelectorValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned> {1});
			
			for (size_t i = 0; i < catchClauses.size(); i++) {
				const auto catchClause = catchClauses[i];
				const auto executeCatchBB = irEmitter_.createBasicBlock("executeCatch");
				const auto tryNextCatchBB = irEmitter_.createBasicBlock("tryNextCatch");
				
				// Call llvm.eh.typeid.for intrinsic to get
				// the selector for the catch type.
				const auto intrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(),
				                                                       llvm::Intrinsic::eh_typeid_for,
				                                                       std::vector<llvm::Type*> {});
				const auto castedCatchTypeInfo = ConstantGenerator(module).getPointerCast(catchTypeList[i], TypeGenerator(module).getPtrType());
				const auto catchSelectorValue = irEmitter_.emitCall(intrinsic->getFunctionType(),
				                                                    intrinsic,
				                                                    std::vector<llvm::Value*> {castedCatchTypeInfo});
				
				// Check thrown selector against catch selector.
				const auto compareResult = function.getBuilder().CreateICmpEQ(catchSelectorValue, throwSelectorValue);
				irEmitter_.emitCondBranch(compareResult, executeCatchBB, tryNextCatchBB);
				
				// If matched, execute catch block and then continue normal execution.
				{
					irEmitter_.selectBasicBlock(executeCatchBB);
					llvm::Value* const getPtrArgs[] = { thrownExceptionValue };
					const auto exceptionPtrFunction = getExceptionPtrFunction(module);
					const auto exceptionPtrValue = irEmitter_.emitCall(exceptionPtrFunction->getFunctionType(),
					                                                   exceptionPtrFunction,
					                                                   getPtrArgs);
					exceptionPtrValue->setDoesNotAccessMemory();
					exceptionPtrValue->setDoesNotThrow();
					
					assert(catchClause->var().isNamed());
					function.getLocalVarMap().forceInsert(&(catchClause->var()), exceptionPtrValue);
					
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
						
						ScopeEmitter(irEmitter_).emitScope(catchClause->scope());
					}
					
					// Exception was handled, so re-commence normal execution.
					if (!irEmitter_.lastInstructionTerminates()) {
						allTerminate = false;
						irEmitter_.emitBranch(afterCatchBB);
					}
				}
				
				irEmitter_.selectBasicBlock(tryNextCatchBB);
			}
			
			// If not matched, keep unwinding.
			genUnwind(function, UnwindStateThrow);
			
			if (!allTerminate) {
				irEmitter_.selectBasicBlock(afterCatchBB);
			} else {
				afterCatchBB->eraseFromParent();
			}
		}
		
		void StatementEmitter::emitThrow(const AST::Value& value) {
			auto& function = irEmitter_.function();
			auto& module = irEmitter_.module();
			ValueEmitter valueEmitter(irEmitter_);
			
			const auto throwType = value.type();
			
			const auto exceptionValue = valueEmitter.emitValue(value);
			
			// Allocate space for exception.
			const auto allocateFunction = getExceptionAllocateFunction(module);
			const auto exceptionValueSize = genSizeOf(function, throwType);
			llvm::Value* const exceptArgs[] = { exceptionValueSize };
			const auto allocatedException = irEmitter_.emitCall(allocateFunction->getFunctionType(),
			                                                    allocateFunction, exceptArgs);
			
			// Store value into allocated space.
			irEmitter_.emitMoveStore(exceptionValue, allocatedException, throwType);
			
			// Call 'throw' function.
			const auto throwFunction = getExceptionThrowFunction(module);
			const auto throwTypeInfo = genThrowInfo(module, throwType->getObjectType());
			const auto nullPtr = ConstantGenerator(module).getNull(TypeGenerator(module).getPtrType());
			llvm::Value* const args[] = { allocatedException, throwTypeInfo, nullPtr };
			
			if (anyUnwindActions(function, UnwindStateThrow)) {
				// Create throw and nothrow paths.
				const auto noThrowPath = irEmitter_.createBasicBlock("");
				const auto throwPath = genLandingPad(function, UnwindStateThrow);
				const auto throwInvoke = irEmitter_.emitInvoke(throwFunction->getFunctionType(),
				                                               throwFunction,
				                                               noThrowPath,
				                                               throwPath,
				                                               args);
				throwInvoke->setDoesNotReturn();
				
				// 'throw' function should never return normally.
				irEmitter_.selectBasicBlock(noThrowPath);
				irEmitter_.emitUnreachable();
			} else {
				const auto callInst = irEmitter_.emitCall(throwFunction->getFunctionType(),
				                                          throwFunction,
				                                          args);
				callInst->setDoesNotReturn();
				
				// 'throw' function should never return normally.
				irEmitter_.emitUnreachable();
			}
		}
		
		void StatementEmitter::emitRethrow() {
			auto& function = irEmitter_.function();
			auto& module = irEmitter_.module();
			
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
				const auto noThrowPath = irEmitter_.createBasicBlock("");
				const auto throwPath = genLandingPad(function, UnwindStateRethrow);
				const auto throwInvoke = irEmitter_.emitInvoke(rethrowFunction->getFunctionType(),
				                                               rethrowFunction,
				                                               noThrowPath,
				                                               throwPath, args);
				throwInvoke->setDoesNotReturn();
				
				// 'rethrow' function should never return normally.
				irEmitter_.selectBasicBlock(noThrowPath);
				irEmitter_.emitUnreachable();
			} else {
				const auto callInst = irEmitter_.emitCall(rethrowFunction->getFunctionType(),
				                                          rethrowFunction, args);
				callInst->setDoesNotReturn();
				
				// 'rethrow' function should never return normally.
				irEmitter_.emitUnreachable();
			}
		}
		
		void StatementEmitter::emitScopeExit(const String& stateString,
		                                     AST::Scope& scope) {
			auto& function = irEmitter_.function();
			
			ScopeExitState state = SCOPEEXIT_ALWAYS;
			
			if (stateString == "exit") {
				state = SCOPEEXIT_ALWAYS;
			} else if (stateString == "success") {
				state = SCOPEEXIT_SUCCESS;
			} else if (stateString == "failure") {
				state = SCOPEEXIT_FAILURE;
			}
			
			function.pushUnwindAction(UnwindAction::ScopeExit(state, &scope));
		}
		
		void StatementEmitter::emitBreak() {
			genUnwind(irEmitter_.function(), UnwindStateBreak);
		}
		
		void StatementEmitter::emitContinue() {
			genUnwind(irEmitter_.function(), UnwindStateContinue);
		}
		
		void StatementEmitter::emitAssert(const AST::Value& value,
		                                  const String& assertName) {
			auto& module = irEmitter_.module();
			ValueEmitter valueEmitter(irEmitter_);
			
			const auto failBB = irEmitter_.createBasicBlock("assertFail");
			const auto successBB = irEmitter_.createBasicBlock("assertSuccess");
			
			const auto boolCondition = valueEmitter.emitValue(value);
			const auto conditionValue = irEmitter_.emitBoolToI1(boolCondition);
			irEmitter_.emitCondBranch(conditionValue, successBB, failBB);
			
			irEmitter_.selectBasicBlock(failBB);
			
			if (!module.buildOptions().unsafe) {
				const auto arrayType = TypeGenerator(module).getArrayType(TypeGenerator(module).getI8Type(),
				                                                          assertName.size() + 1);
				const auto constArray = ConstantGenerator(module).getString(assertName);
				const auto globalArray = module.createConstGlobal(module.getCString("assert_name_constant"),
				                                                  arrayType, llvm::GlobalValue::InternalLinkage,
				                                                  constArray);
				globalArray->setAlignment(1);
				const auto stringGlobal = irEmitter_.emitConstInBoundsGEP2_32(arrayType,
				                                                              globalArray,
				                                                              0, 0);
				
				const auto assertFailedFunction = getAssertFailedFunction(module);
				llvm::Value* const args[] = { stringGlobal };
				const auto callInst = irEmitter_.emitCall(assertFailedFunction->getFunctionType(),
				                                          assertFailedFunction, args);
				callInst->setDoesNotThrow();
				callInst->setDoesNotReturn();
			}
			
			// Still want to create the conditional branch and unreachable
			// in 'unsafe' mode, since this is a hint to the optimiser.
			irEmitter_.emitUnreachable();
			
			irEmitter_.selectBasicBlock(successBB);
		}
		
		void StatementEmitter::emitAssertNoExcept(const AST::Scope& scope) {
			// Basically a no-op.
			ScopeEmitter(irEmitter_).emitScope(scope);
		}
		
		void StatementEmitter::emitUnreachable() {
			auto& module = irEmitter_.module();
			
			if (!module.buildOptions().unsafe) {
				const auto unreachableFailedFunction = getUnreachableFailedFunction(module);
				const auto callInst = irEmitter_.emitCall(unreachableFailedFunction->getFunctionType(),
				                                          unreachableFailedFunction,
				                                          std::vector<llvm::Value*>());
				callInst->setDoesNotThrow();
				callInst->setDoesNotReturn();
			}
			irEmitter_.emitUnreachable();
		}
		
	}
	
}

