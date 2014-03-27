#include <assert.h>

#include <stdexcept>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/ControlFlow.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genScope(Function& function, const SEM::Scope& scope) {
			LifetimeScope lifetimeScope(function);
			
			for (const auto localVar: scope.localVariables()) {
				genVarAlloca(function, localVar);
			}
			
			for (const auto statement: scope.statements()) {
				genStatement(function, statement);
			}
		}
		
		static llvm::Value* encodeReturnValue(Function& function, llvm::Value* value, llvm_abi::Type type) {
			std::vector<llvm_abi::Type> abiTypes;
			abiTypes.push_back(std::move(type));
			return function.module().abi().encodeValues(function.getEntryBuilder(), function.getBuilder(), {value}, abiTypes).at(0);
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
					const auto mergeBB = function.createBasicBlock("ifMerge");
					
					assert(!statement->getIfClauseList().empty());
					
					for (const auto ifClause: statement->getIfClauseList()) {
						const auto conditionValue = genValue(function, ifClause->condition());
						
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
						
						const auto tagValue = ConstantGenerator(function.module()).getI8(tag);
						const auto caseBB = function.createBasicBlock("switchCase");
						
						switchInstruction->addCase(tagValue, caseBB);
						
						function.selectBasicBlock(caseBB);
						
						const auto unionValueType = genType(function.module(), caseType);
						const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionValuePtr, unionValueType->getPointerTo());
						
						{
							LifetimeScope lifetimeScope(function);
							genVarAlloca(function, switchCase->var());
							genVarInitialise(function, switchCase->var(), castedUnionValuePtr);
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
						LifetimeScope conditionLifetimeScope(function);
						condition = genValue(function, statement->getLoopCondition());
					}
					
					function.getBuilder().CreateCondBr(condition, loopIterationBB, loopEndBB);
					
					// Create loop contents.
					function.selectBasicBlock(loopIterationBB);
					
					{
						ControlFlowScope controlFlowScope(function.unwindStack(), loopEndBB, loopAdvanceBB);
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
					
					if (statement->getReturnValue() != nullptr
						&& !statement->getReturnValue()->type()->isVoid()) {
						if (function.getArgInfo().hasReturnVarArgument()) {
							const auto returnValue = genValue(function, statement->getReturnValue());
							
							// Store the return value into the return value pointer.
							genStore(function, returnValue, function.getReturnVar(), statement->getReturnValue()->type());
							
							// Call all destructors.
							genAllScopeDestructorCalls(function);
							
							returnInst = function.getBuilder().CreateRetVoid();
						} else {
							const auto returnValue = genValue(function, statement->getReturnValue());
							
							const auto encodedReturnValue = encodeReturnValue(function, returnValue, genABIType(function.module(), statement->getReturnValue()->type()));
							
							// Call all destructors.
							genAllScopeDestructorCalls(function);
							
							returnInst = function.getBuilder().CreateRet(encodedReturnValue);
						}
					} else {
						// Call all destructors.
						genAllScopeDestructorCalls(function);
						
						returnInst = function.getBuilder().CreateRetVoid();
					}
					
					if (hasDebugInfo) {
						returnInst->setDebugLoc(debugLocation);
					}
					
					// Need a basic block after a return statement in case anything more is generated.
					// This (and any following code) will be removed by dead code elimination.
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
					
					const auto catchBlock = function.createBasicBlock("catch");
					
					// Execute the 'try' scope, pushing the exception
					// handlers onto the unwind stack.
					{
						TryScope tryScope(function.unwindStack(), catchBlock, catchTypeList);
						genScope(function, statement->getTryScope());
					}
					
					const auto afterCatchBlock = function.createBasicBlock("afterCatch");
					
					// No exception thrown; continue normal execution.
					function.getBuilder().CreateBr(afterCatchBlock);
					
					function.selectBasicBlock(catchBlock);
					
					// Load selector of exception thrown.
					const auto exceptionInfo = function.getBuilder().CreateLoad(function.exceptionInfo());
					const auto thrownExceptionValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned>{0});
					const auto throwSelectorValue = function.getBuilder().CreateExtractValue(exceptionInfo, std::vector<unsigned>{1});
					
					for (size_t i = 0; i < statement->getTryCatchList().size(); i++) {
						const auto catchClause = statement->getTryCatchList().at(i);
						const auto executeCatchBlock = function.createBasicBlock("executeCatch");
						const auto tryNextCatchBlock = function.createBasicBlock("tryNextCatch");
						
						// Call llvm.eh.typeid.for intrinsic to get
						// the selector for the catch type.
						const auto intrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::eh_typeid_for, std::vector<llvm::Type*>{});
						const auto castedCatchTypeInfo = ConstantGenerator(module).getPointerCast(catchTypeList.at(i), TypeGenerator(module).getI8PtrType());
						const auto catchSelectorValue = function.getBuilder().CreateCall(intrinsic, std::vector<llvm::Value*>{castedCatchTypeInfo});
						
						// Check thrown selector against catch selector.
						const auto compareResult = function.getBuilder().CreateICmpEQ(catchSelectorValue, throwSelectorValue);
						function.getBuilder().CreateCondBr(compareResult, executeCatchBlock, tryNextCatchBlock);
						
						// If matched, execute catch block and then continue normal execution.
						{
							function.selectBasicBlock(executeCatchBlock);
							const auto catchType = genType(function.module(), catchClause->var()->constructType());
							const auto exceptionDataValue = function.getBuilder().CreateCall(getBeginCatchFunction(module), std::vector<llvm::Value*>{thrownExceptionValue});
							const auto castedExceptionValue = function.getBuilder().CreatePointerCast(exceptionDataValue, catchType->getPointerTo());
							
							assert(catchClause->var()->isBasic());
							function.getLocalVarMap().forceInsert(catchClause->var(), castedExceptionValue);
							genScope(function, catchClause->scope());
							
							// TODO: make sure this gets called if an exception is thrown in the handler!
							function.getBuilder().CreateCall(getEndCatchFunction(module), std::vector<llvm::Value*>{});
							
							// Exception was handled, so re-commence normal execution.
							function.getBuilder().CreateBr(afterCatchBlock);
						}
						
						function.selectBasicBlock(tryNextCatchBlock);
					}
					
					// If not matched, keep unwinding.
					genExceptionUnwind(function);
					
					function.selectBasicBlock(afterCatchBlock);
					break;
				}
				
				case SEM::Statement::THROW: {
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
					genLandingPad(function);
					
					// Basic block for any further instructions generated.
					function.selectBasicBlock(function.createBasicBlock("afterThrow"));
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

