#include <assert.h>

#include <set>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/Support/MakeArray.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertForLoop.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Value GetAssignValue(Context& context, AST::AssignKind assignKind, SEM::Value varValue, SEM::Value operandValue, const Debug::SourceLocation& location) {
			switch (assignKind) {
				case AST::ASSIGN_DIRECT:
					return operandValue;
				case AST::ASSIGN_ADD: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("add"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
				case AST::ASSIGN_SUB: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("subtract"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
				case AST::ASSIGN_MUL: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("multiply"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
				case AST::ASSIGN_DIV: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("divide"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
				case AST::ASSIGN_MOD: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("modulo"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
			}
			
			std::terminate();
		}
		
		class AssertNoExceptAroundNoexceptScopeDiag: public Warning {
		public:
			AssertNoExceptAroundNoexceptScopeDiag() { }
			
			std::string toString() const {
				return "assert noexcept is around scope that is guaranteed to never throw anyway";
			}
			
		};
		
		SEM::Statement ConvertStatementData(Context& context, const AST::Node<AST::Statement>& statement) {
			const auto& location = statement.location();
			
			switch (statement->typeEnum) {
				case AST::Statement::VALUE: {
					auto value = ConvertValue(context, statement->valueStmt.value);
					if (statement->valueStmt.hasVoidCast) {
						if (value.type()->isBuiltInVoid()) {
							throw ErrorException(makeString("Void explicitly ignored in expression '%s' at position %s.",
								value.toString().c_str(), location.toString().c_str()));
						}
						const auto voidType = context.typeBuilder().getVoidType();
						return SEM::Statement::ValueStmt(SEM::Value::Cast(voidType, std::move(value)));
					} else {
						if (!value.type()->isBuiltInVoid()) {
							throw ErrorException(makeString("Non-void value result ignored in expression '%s' at position %s.",
								value.toString().c_str(), location.toString().c_str()));
						}
						return SEM::Statement::ValueStmt(std::move(value));
					}
				}
				case AST::Statement::SCOPE: {
					return SEM::Statement::ScopeStmt(ConvertScope(context, statement->scopeStmt.scope));
				}
				case AST::Statement::IF: {
					const auto boolType = context.typeBuilder().getBoolType();
					
					std::vector<SEM::IfClause*> clauseList;
					for (const auto& astIfClause: *(statement->ifStmt.clauseList)) {
						auto condition = ConvertValue(context, astIfClause->condition);
						auto boolValue = ImplicitCast(context, std::move(condition), boolType, location);
						auto ifTrueScope = ConvertScope(context, astIfClause->scope);
						clauseList.push_back(new SEM::IfClause(std::move(boolValue), std::move(ifTrueScope)));
					}
					
					auto elseScope = ConvertScope(context, statement->ifStmt.elseScope);
					
					return SEM::Statement::If(std::move(clauseList), std::move(elseScope));
				}
				case AST::Statement::SWITCH: {
					auto value = ConvertValue(context, statement->switchStmt.value);
					
					std::map<const SEM::TypeInstance*, const SEM::Type*> switchCaseTypes;
					
					std::vector<SEM::SwitchCase*> caseList;
					for (const auto& astCase: *(statement->switchStmt.caseList)) {
						std::unique_ptr<SEM::SwitchCase> semCase(new SEM::SwitchCase());
						
						{
							PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::SwitchCase(*semCase));
							
							semCase->setVar(ConvertVar(context, Debug::VarInfo::VAR_LOCAL, astCase->var));
							semCase->setScope(ConvertScope(context, astCase->scope));
						}
						
						const auto caseType = semCase->var().constructType();
						const auto insertResult = switchCaseTypes.insert(std::make_pair(caseType->getObjectType(), caseType));
						
						// Check for duplicate cases.
						if (!insertResult.second) {
							throw ErrorException(makeString("Duplicate switch case for type '%s' at position %s.",
								(insertResult.first->first)->refToString().c_str(),
								location.toString().c_str()));
						}
						
						caseList.push_back(semCase.release());
					}
					
					if (caseList.empty()) {
						throw ErrorException(makeString("Switch statement must contain at least one case at position %s.",
							location.toString().c_str()));
					}
					
					const auto firstSwitchTypeIterator = switchCaseTypes.begin();
					
					// Check that all switch cases are based
					// on the same union datatype.
					const auto switchTypeInstance = firstSwitchTypeIterator->first->parentTypeInstance();
					for (auto caseTypePair: switchCaseTypes) {
						const auto caseTypeInstance = caseTypePair.first;
						const auto caseTypeInstanceParent = caseTypeInstance->parentTypeInstance();
						
						if (caseTypeInstanceParent == nullptr) {
							throw ErrorException(makeString("Switch case type '%s' is not a member of a union datatype at position %s.",
								caseTypeInstance->refToString().c_str(),
								location.toString().c_str()));
						}
						
						if (caseTypeInstanceParent != switchTypeInstance) {
							throw ErrorException(makeString("Switch case type '%s' does not share the same parent as type '%s' at position %s.",
								caseTypeInstance->refToString().c_str(),
								(firstSwitchTypeIterator->first)->refToString().c_str(),
								location.toString().c_str()));
						}
					}
					
					const auto substitutedSwitchType = switchTypeInstance->selfType()->substitute(firstSwitchTypeIterator->second->generateTemplateVarMap());
					
					// Case value to switch type.
					auto castValue = ImplicitCast(context, std::move(value), substitutedSwitchType,
					                              statement->switchStmt.value.location());
					
					const auto& astDefaultCase = statement->switchStmt.defaultCase;
					const bool hasDefaultCase = astDefaultCase->hasScope;
					
					std::vector<SEM::TypeInstance*> unhandledCases;
					
					// Check whether all cases are handled.
					for (auto variantTypeInstance: switchTypeInstance->variants()) {
						if (switchCaseTypes.find(variantTypeInstance) == switchCaseTypes.end()) {
							unhandledCases.push_back(variantTypeInstance);
						}
					}
					
					if (hasDefaultCase) {
						if (unhandledCases.empty()) {
							throw ErrorException(makeString("All cases are handled in switch with (unused) default case at position %s.",
								location.toString().c_str()));
						}
						
						auto defaultScope = ConvertScope(context, astDefaultCase->scope);
						return SEM::Statement::Switch(std::move(castValue), caseList, std::move(defaultScope));
					} else {
						if (!unhandledCases.empty()) {
							throw ErrorException(makeString("Union datatype member '%s' not handled in switch at position %s.",
								unhandledCases.front()->refToString().c_str(), location.toString().c_str()));
						}
						
						return SEM::Statement::Switch(std::move(castValue), caseList, nullptr);
					}
				}
				case AST::Statement::WHILE: {
					auto condition = ConvertValue(context, statement->whileStmt.condition);
					
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Loop());
					
					auto iterationScope = ConvertScope(context, statement->whileStmt.whileTrue);
					auto advanceScope = SEM::Scope::Create();
					auto loopCondition = ImplicitCast(context, std::move(condition), context.typeBuilder().getBoolType(), location);
					return SEM::Statement::Loop(std::move(loopCondition), std::move(iterationScope), std::move(advanceScope));
				}
				case AST::Statement::FOR: {
					const auto& forStmt = statement->forStmt;
					auto loopScope = ConvertForLoop(context, forStmt.typeVar, forStmt.initValue, forStmt.scope);
					return SEM::Statement::ScopeStmt(std::move(loopScope));
				}
				case AST::Statement::TRY: {
					std::unique_ptr<SEM::Scope> tryScope;
					
					{
						PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TryScope());
						tryScope = ConvertScope(context, statement->tryStmt.scope);
						
						const auto exitStates = tryScope->exitStates();
						assert(!exitStates.hasRethrowExit());
						
						if (!exitStates.hasThrowExit()) {
							throw ErrorException(makeString("Try statement wraps a scope that cannot throw, at position %s.",
								location.toString().c_str()));
						}
					}
					
					std::vector<SEM::CatchClause*> catchList;
					
					for (const auto& astCatch: *(statement->tryStmt.catchList)) {
						std::unique_ptr<SEM::CatchClause> semCatch(new SEM::CatchClause());
						
						PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::CatchClause(*semCatch));
						
						const auto& astVar = astCatch->var;
						
						if (!astVar->isNamed()) {
							throw ErrorException(makeString("Try statement catch clauses may only "
								"contain named variables (no pattern matching) at position %s.",
								location.toString().c_str()));
						}
						
						// Special case handling for catch variables,
						// since they don't use lvalues.
						const auto varType = ConvertType(context, astVar->namedType());
						if (!varType->isException()) {
							throw ErrorException(makeString("Type '%s' is not an exception type and therefore "
								"cannot be used in a catch clause at position %s.",
								varType->toString().c_str(), location.toString().c_str()));
						}
						
						auto semVar = SEM::Var::Basic(varType, varType);
						attachVar(context, astVar->name(), astVar, *semVar, Debug::VarInfo::VAR_LOCAL);
						
						semCatch->setVar(std::move(semVar));
						semCatch->setScope(ConvertScope(context, astCatch->scope));
						
						catchList.push_back(semCatch.release());
					}
					
					return SEM::Statement::Try(std::move(tryScope), catchList);
				}
				case AST::Statement::SCOPEEXIT: {
					const auto& scopeExitState = statement->scopeExitStmt.state;
					if (scopeExitState != "exit" && scopeExitState != "success" && scopeExitState != "failure") {
						throw ErrorException(makeString("Unknown scope-exit state '%s' at position %s.",
								scopeExitState.c_str(), location.toString().c_str()));
					}
					
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::ScopeAction(scopeExitState));
					
					auto scopeExitScope = ConvertScope(context, statement->scopeExitStmt.scope);
					const auto exitStates = scopeExitScope->exitStates();
					
					assert(!exitStates.hasReturnExit());
					assert(!exitStates.hasBreakExit());
					assert(!exitStates.hasContinueExit());
					
					// scope(success) is allowed to throw.
					if (scopeExitState != "success" && (exitStates.hasThrowExit() || exitStates.hasRethrowExit())) {
						throw ErrorException(makeString("Scope exit action (for state '%s') can throw, at position %s.",
								scopeExitState.c_str(), location.toString().c_str()));
					}
					
					return SEM::Statement::ScopeExit(scopeExitState, std::move(scopeExitScope));
				}
				case AST::Statement::VARDECL: {
					const auto& astTypeVarNode = statement->varDecl.typeVar;
					const auto& astInitialValueNode = statement->varDecl.value;
					
					auto semValue = ConvertValue(context, astInitialValueNode);
					
					const auto varDeclType = getVarType(context, astTypeVarNode, semValue.type());
					
					// Cast the initialise value to the variable's type.
					auto semInitialiseValue = ImplicitCast(context, std::move(semValue), varDeclType, location);
					
					// Convert the AST type var.
					const auto varType = semInitialiseValue.type();
					auto semVar = ConvertInitialisedVar(context, astTypeVarNode, varType);
					assert(!semVar->isAny());
					
					// Add the variable to the SEM scope.
					auto& semScope = context.scopeStack().back().scope();
					
					const auto varPtr = semVar.get();
					semScope.variables().push_back(semVar.release());
					
					// Generate the initialise statement.
					return SEM::Statement::InitialiseStmt(varPtr, std::move(semInitialiseValue));
				}
				case AST::Statement::ASSIGN: {
					const auto assignKind = statement->assignStmt.assignKind;
					auto semVarValue = derefValue(ConvertValue(context, statement->assignStmt.var));
					auto semOperandValue = ConvertValue(context, statement->assignStmt.value);
					
					if (!getDerefType(semVarValue.type())->isLval()) {
						throw ErrorException(makeString("Can't assign to non-lval type '%s' at position %s.",
							semVarValue.type()->toString().c_str(),
							location.toString().c_str()));
					}
					
					// TODO: fix this to not copy the value!
					auto semAssignValue = GetAssignValue(context, assignKind, semVarValue.copy(), std::move(semOperandValue), location);
					auto opMethod = GetSpecialMethod(context, derefOrBindValue(context, std::move(semVarValue)), context.getCString("assign"), location);
					return SEM::Statement::ValueStmt(CallValue(context, std::move(opMethod), makeHeapArray(std::move(semAssignValue)), location));
				}
				case AST::Statement::INCREMENT: {
					auto semOperandValue = ConvertValue(context, statement->incrementStmt.value);
					auto opMethod = GetMethod(context, std::move(semOperandValue), context.getCString("increment"), location);
					auto opResult = CallValue(context, std::move(opMethod), { }, location);
					
					if (opResult.type()->isBuiltInVoid()) {
						return SEM::Statement::ValueStmt(std::move(opResult));
					} else {
						// Automatically cast to void if necessary.
						const auto voidType = context.typeBuilder().getVoidType();
						auto voidCastedValue = SEM::Value::Cast(voidType, std::move(opResult));
						return SEM::Statement::ValueStmt(std::move(voidCastedValue));
					}
				}
				case AST::Statement::DECREMENT: {
					auto semOperandValue = ConvertValue(context, statement->decrementStmt.value);
					auto opMethod = GetMethod(context, std::move(semOperandValue), context.getCString("decrement"), location);
					auto opResult = CallValue(context, std::move(opMethod), { }, location);
					
					if (opResult.type()->isBuiltInVoid()) {
						return SEM::Statement::ValueStmt(std::move(opResult));
					} else {
						// Automatically cast to void if necessary.
						const auto voidType = context.typeBuilder().getVoidType();
						auto voidCastedValue = SEM::Value::Cast(voidType, std::move(opResult));
						return SEM::Statement::ValueStmt(std::move(voidCastedValue));
					}
				}
				case AST::Statement::RETURNVOID: {
					// Void return statement (i.e. return;)
					if (!getParentFunctionReturnType(context.scopeStack())->isBuiltInVoid()) {
						throw ErrorException(makeString("Cannot return void in function '%s' with non-void return type at position %s.",
							lookupParentFunction(context.scopeStack())->name().toString().c_str(),
							location.toString().c_str()));
					}
					
					return SEM::Statement::ReturnVoid();
				}
				case AST::Statement::RETURN: {
					assert(statement->returnStmt.value.get() != nullptr);
					
					// Check this is not being used inside a scope action.
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						if (element.isScopeAction()) {
							throw ErrorException(makeString("Cannot 'return' in scope action at position %s.",
								location.toString().c_str()));
						}
					}
					
					// Can't return in a function that returns void.
					if (getParentFunctionReturnType(context.scopeStack())->isBuiltInVoid()) {
						throw ErrorException(makeString("Cannot return value in function '%s' with void return type at position %s.",
							lookupParentFunction(context.scopeStack())->name().toString().c_str(),
							location.toString().c_str()));
					}
					
					auto semValue = ConvertValue(context, statement->returnStmt.value);
					
					// Can't return void.
					if (semValue.type()->isBuiltInVoid()) {
						throw ErrorException(makeString("Cannot return void in function '%s' with non-void return type at position %s.",
							lookupParentFunction(context.scopeStack())->name().toString().c_str(),
							location.toString().c_str()));
					}
					
					// Cast the return value to the function's
					// specified return type.
					auto castValue = ImplicitCast(context, std::move(semValue), getParentFunctionReturnType(context.scopeStack()), location);
					return SEM::Statement::Return(std::move(castValue));
				}
				case AST::Statement::THROW: {
					// Check this is not being used inside a scope action
					// (apart from inside scope(success), which is allowed).
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						if (element.isScopeAction() && element.scopeActionState() != "success") {
							throw ErrorException(makeString("Cannot 'throw' in scope action with state '%s' at position %s.",
								element.scopeActionState().c_str(), location.toString().c_str()));
						}
					}
					
					auto semValue = ConvertValue(context, statement->throwStmt.value);
					if (!semValue.type()->isObject() || !semValue.type()->getObjectType()->isException()) {
						throw ErrorException(makeString("Cannot throw non-exception value '%s' at position %s.",
								semValue.toString().c_str(), location.toString().c_str()));
					}
					return SEM::Statement::Throw(std::move(semValue));
				}
				case AST::Statement::RETHROW: {
					// Check this is being used inside a catch clause, and
					// is not inside a try clause or a scope action.
					bool foundCatchClause = false;
					
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						
						if (element.isCatchClause()) {
							foundCatchClause = true;
						} else if (element.isTryScope() && !foundCatchClause) {
							throw ErrorException(makeString("Cannot re-throw exception in try scope (within the relevant catch clause) at position %s.",
								location.toString().c_str()));
						} else if (element.isScopeAction() && element.scopeActionState() != "success") {
							throw ErrorException(makeString("Cannot re-throw exception in scope action with state '%s' at position %s.",
								element.scopeActionState().c_str(), location.toString().c_str()));
						}
					}
					
					if (!foundCatchClause) {
						throw ErrorException(makeString("Cannot re-throw exception outside of catch clause at position %s.",
							location.toString().c_str()));
					}
					
					return SEM::Statement::Rethrow();
				}
				case AST::Statement::BREAK: {
					// Check this is being used inside a loop, and
					// would not leave a scope-exit action.
					bool foundLoop = false;
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						if (element.isLoop()) {
							foundLoop = true;
							break;
						} else if (element.isScopeAction()) {
							throw ErrorException(makeString("Cannot 'break' from scope exit action at position %s.",
								location.toString().c_str()));
						}
					}
					
					if (!foundLoop) {
						throw ErrorException(makeString("Cannot 'break' outside any control flow statements at position %s.",
							location.toString().c_str()));
					}
					
					return SEM::Statement::Break();
				}
				case AST::Statement::CONTINUE: {
					// Check this is being used inside a loop, and
					// would not leave a scope-exit action.
					bool foundLoop = false;
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						if (element.isLoop()) {
							foundLoop = true;
							break;
						} else if (element.isScopeAction()) {
							throw ErrorException(makeString("Cannot 'continue' in scope exit action at position %s.",
								location.toString().c_str()));
						}
					}
					
					if (!foundLoop) {
						throw ErrorException(makeString("Cannot 'continue' outside any control flow statements at position %s.",
							location.toString().c_str()));
					}
					
					return SEM::Statement::Continue();
				}
				case AST::Statement::ASSERT: {
					assert(statement->assertStmt.value.get() != nullptr);
					
					const auto boolType = context.typeBuilder().getBoolType();
					auto condition = ConvertValue(context, statement->assertStmt.value);
					auto boolValue = ImplicitCast(context, std::move(condition), boolType, location);
					return SEM::Statement::Assert(std::move(boolValue), statement->assertStmt.name);
				}
				case AST::Statement::ASSERTNOEXCEPT: {
					auto scope = ConvertScope(context, statement->assertNoExceptStmt.scope);
					const auto scopeExitStates = scope->exitStates();
					if (!scopeExitStates.hasThrowExit() && !scopeExitStates.hasRethrowExit()) {
						context.issueDiag(AssertNoExceptAroundNoexceptScopeDiag(),
						                  location);
					}
					
					return SEM::Statement::AssertNoExcept(std::move(scope));
				}
				case AST::Statement::UNREACHABLE: {
					return SEM::Statement::Unreachable();
				}
			}
			
			std::terminate();
		}
		
		Debug::StatementInfo makeStatementInfo(const AST::Node<AST::Statement>& astStatementNode) {
			Debug::StatementInfo statementInfo;
			statementInfo.location = astStatementNode.location();
			return statementInfo;
		}
		
		SEM::Statement ConvertStatement(Context& context, const AST::Node<AST::Statement>& astStatementNode) {
			auto semStatement = ConvertStatementData(context, astStatementNode);
			semStatement.setDebugInfo(makeStatementInfo(astStatementNode));
			return semStatement;
		}
		
	}
	
}


