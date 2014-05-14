#include <assert.h>

#include <set>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertForLoop.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		bool WillStatementReturn(SEM::Statement* statement) {
			switch(statement->kind()) {
				case SEM::Statement::VALUE: {
					return false;
				}
				case SEM::Statement::SCOPE: {
					return WillScopeReturn(statement->getScope());
				}
				case SEM::Statement::INITIALISE: {
					return false;
				}
				case SEM::Statement::IF: {
					for (const auto ifClause: statement->getIfClauseList()) {
						if (!WillScopeReturn(ifClause->scope())) {
							return false;
						}
					}
					return WillScopeReturn(statement->getIfElseScope());
				}
				case SEM::Statement::SWITCH: {
					for (auto switchCase: statement->getSwitchCaseList()) {
						if (!WillScopeReturn(switchCase->scope())) {
							return false;
						}
					}
					return true;
				}
				case SEM::Statement::LOOP: {
					return WillScopeReturn(statement->getLoopIterationScope());
				}
				case SEM::Statement::TRY: {
					// TODO: also consider catch blocks?
					return WillScopeReturn(statement->getTryScope());
				}
				case SEM::Statement::SCOPEEXIT: {
					return false;
				}
				case SEM::Statement::RETURN: {
					return true;
				}
				case SEM::Statement::THROW: {
					// TODO: doesn't seem correct...
					return true;
				}
				case SEM::Statement::BREAK: {
					// TODO: doesn't seem correct...
					return true;
				}
				case SEM::Statement::CONTINUE: {
					return false;
				}
				default: {
					throw std::runtime_error("Unknown statement kind.");
				}
			}
		}
		
		bool CanStatementThrow(SEM::Statement* statement) {
			switch(statement->kind()) {
				case SEM::Statement::VALUE: {
					return CanValueThrow(statement->getValue());
				}
				case SEM::Statement::SCOPE: {
					return CanScopeThrow(statement->getScope());
				}
				case SEM::Statement::INITIALISE: {
					return false;
				}
				case SEM::Statement::IF: {
					for (const auto ifClause: statement->getIfClauseList()) {
						if (CanValueThrow(ifClause->condition()) || CanScopeThrow(ifClause->scope())) {
							return true;
						}
					}
					return CanScopeThrow(statement->getIfElseScope());
				}
				case SEM::Statement::SWITCH: {
					for (auto switchCase: statement->getSwitchCaseList()) {
						if (CanScopeThrow(switchCase->scope())) {
							return true;
						}
					}
					return CanValueThrow(statement->getSwitchValue());
				}
				case SEM::Statement::LOOP: {
					return CanValueThrow(statement->getLoopCondition()) ||
						CanScopeThrow(statement->getLoopIterationScope()) ||
						CanScopeThrow(statement->getLoopAdvanceScope());
				}
				case SEM::Statement::TRY: {
					for (auto catchClause: statement->getTryCatchList()) {
						if (CanScopeThrow(catchClause->scope())) {
							return true;
						}
					}
					return CanScopeThrow(statement->getTryScope());
				}
				case SEM::Statement::SCOPEEXIT: {
					// TODO: handle scope(success) which is allowed to throw.
					return false;
				}
				case SEM::Statement::RETURN: {
					return false;
				}
				case SEM::Statement::THROW: {
					return true;
				}
				case SEM::Statement::BREAK: {
					return false;
				}
				case SEM::Statement::CONTINUE: {
					return false;
				}
				default: {
					throw std::runtime_error("Unknown statement kind.");
				}
			}
		}
		
		SEM::Statement* ConvertStatementData(Context& context, const AST::Node<AST::Statement>& statement) {
			const auto& location = statement.location();
			
			switch (statement->typeEnum) {
				case AST::Statement::VALUE: {
					const auto value = ConvertValue(context, statement->valueStmt.value);
					if (statement->valueStmt.hasVoidCast) {
						if (value->type()->isVoid()) {
							throw ErrorException(makeString("Void explicitly ignored in expression '%s' at position %s.",
								value->toString().c_str(), location.toString().c_str()));
						}
						return SEM::Statement::ValueStmt(SEM::Value::Cast(SEM::Type::Void(), value));
					} else {
						if (!value->type()->isVoid()) {
							throw ErrorException(makeString("Non-void value result ignored in expression '%s' at position %s.",
								value->toString().c_str(), location.toString().c_str()));
						}
						return SEM::Statement::ValueStmt(value);
					}
				}
				case AST::Statement::SCOPE: {
					return SEM::Statement::ScopeStmt(ConvertScope(context, statement->scopeStmt.scope));
				}
				case AST::Statement::IF: {
					const auto boolType = getBuiltInType(context, "bool");
					
					std::vector<SEM::IfClause*> clauseList;
					for (const auto& astIfClause: *(statement->ifStmt.clauseList)) {
						const auto condition = ConvertValue(context, astIfClause->condition);
						const auto boolValue = ImplicitCast(condition, boolType->selfType(), location);
						const auto ifTrueScope = ConvertScope(context, astIfClause->scope);
						clauseList.push_back(new SEM::IfClause(boolValue, ifTrueScope));
					}
					
					const auto elseScope = ConvertScope(context, statement->ifStmt.elseScope);
					
					return SEM::Statement::If(clauseList, elseScope);
				}
				case AST::Statement::SWITCH: {
					const auto value = ConvertValue(context, statement->switchStmt.value);
					
					std::set<SEM::TypeInstance*> switchCaseTypeInstances;
					
					std::vector<SEM::SwitchCase*> caseList;
					for (const auto& astCase: *(statement->switchStmt.caseList)) {
						auto semCase = new SEM::SwitchCase();
						auto switchCaseNode = Node::SwitchCase(astCase, semCase);
						NodeContext switchCaseContext(context, "#switchcase", switchCaseNode);
						
						const bool isMember = false;
						semCase->setVar(ConvertVar(switchCaseContext, isMember, astCase->var));
						semCase->setScope(ConvertScope(switchCaseContext, astCase->scope));
						
						caseList.push_back(semCase);
						
						const auto insertResult = switchCaseTypeInstances.insert(semCase->var()->constructType()->getObjectType());
						
						// Check for duplicate cases.
						if (!insertResult.second) {
							throw ErrorException(makeString("Duplicate switch case for type '%s' at position %s.",
								(*(insertResult.first))->refToString().c_str(),
								location.toString().c_str()));
						}
					}
					
					if (caseList.empty()) {
						throw ErrorException(makeString("Switch statement must contain at least one case at position %s.",
							location.toString().c_str()));
					}
					
					// Check that all switch cases are based
					// on the same union datatype.
					const auto switchTypeInstance = (*(switchCaseTypeInstances.begin()))->parent();
					for (auto caseTypeInstance: switchCaseTypeInstances) {
						auto caseTypeInstanceParent = caseTypeInstance->parent();
						
						if (caseTypeInstanceParent == nullptr) {
							throw ErrorException(makeString("Switch case type '%s' is not a member of a union datatype at position %s.",
								caseTypeInstance->refToString().c_str(),
								location.toString().c_str()));
						}
						
						if (caseTypeInstanceParent != switchTypeInstance) {
							throw ErrorException(makeString("Switch case type '%s' does not share the same parent as type '%s' at position %s.",
								caseTypeInstance->refToString().c_str(),
								(*(switchCaseTypeInstances.begin()))->refToString().c_str(),
								location.toString().c_str()));
						}
					}
					
					// TODO: implement 'default' case.
					const bool hasDefaultCase = false;
					
					if (!hasDefaultCase) {
						// Check all cases are handled.
						for (auto variantTypeInstance: switchTypeInstance->variants()) {
							if (switchCaseTypeInstances.find(variantTypeInstance) == switchCaseTypeInstances.end()) {
								throw ErrorException(makeString("Union datatype member '%s' not handled in switch at position %s.",
									variantTypeInstance->refToString().c_str(), location.toString().c_str()));
							}
						}
					}
					
					// Case value to switch type.
					// TODO: fix the template arguments for the switch type.
					const auto castValue = ImplicitCast(value, switchTypeInstance->selfType(), location);
					
					return SEM::Statement::Switch(castValue, caseList);
				}
				case AST::Statement::WHILE: {
					const auto condition = ConvertValue(context, statement->whileStmt.condition);
					const auto iterationScope = ConvertScope(context, statement->whileStmt.whileTrue);
					const auto advanceScope = new SEM::Scope();
					const auto loopCondition = ImplicitCast(condition, getBuiltInType(context, "bool")->selfType(), location);
					return SEM::Statement::Loop(loopCondition, iterationScope, advanceScope);
				}
				case AST::Statement::FOR: {
					const auto& forStmt = statement->forStmt;
					const auto loopScope = ConvertForLoop(context, forStmt.typeVar, forStmt.initValue, forStmt.scope);
					return SEM::Statement::ScopeStmt(loopScope);
				}
				case AST::Statement::TRY: {
					const auto tryScope = ConvertScope(context, statement->tryStmt.scope);
					
					std::vector<SEM::CatchClause*> catchList;
					
					for (const auto& astCatch: *(statement->tryStmt.catchList)) {
						auto semCatch = new SEM::CatchClause();
						auto catchClauseNode = Node::CatchClause(astCatch, semCatch);
						NodeContext catchClauseContext(context, "#catchclause", catchClauseNode);
						
						const auto& astVar = astCatch->var;
						
						if (astVar->kind != AST::TypeVar::NAMEDVAR) {
							throw ErrorException(makeString("Try statement catch clauses may only "
								"contain named variables (no pattern matching) at position %s.",
								location.toString().c_str()));
						}
						
						// Special case handling for catch variables,
						// since they don't use lvalues.
						const auto varType = ConvertType(context, astVar->namedVar.type);
						if (!varType->isException()) {
							throw ErrorException(makeString("Type '%s' is not an exception type and therefore "
								"cannot be used in a catch clause at position %s.",
								varType->toString().c_str(), location.toString().c_str()));
						}
						
						const auto semVar = SEM::Var::Basic(varType, varType);
						attachVar(catchClauseContext, astVar->namedVar.name, astVar, semVar);
						
						semCatch->setVar(semVar);
						semCatch->setScope(ConvertScope(catchClauseContext, astCatch->scope));
						
						catchList.push_back(semCatch);
					}
					
					return SEM::Statement::Try(tryScope, catchList);
				}
				case AST::Statement::SCOPEEXIT: {
					const auto& scopeExitState = statement->scopeExitStmt.state;
					if (scopeExitState != "exit" && scopeExitState != "success" && scopeExitState != "failure") {
						throw ErrorException(makeString("Unknown scope-exit state '%s' at position %s.",
								scopeExitState.c_str(), location.toString().c_str()));
					}
					
					const auto scopeExitScope = ConvertScope(context, statement->scopeExitStmt.scope);
					
					if (CanScopeThrow(*scopeExitScope)) {
						throw ErrorException(makeString("scope(%s) can throw, at position %s.",
								scopeExitState.c_str(), location.toString().c_str()));
					}
					
					return SEM::Statement::ScopeExit(scopeExitState, scopeExitScope);
				}
				case AST::Statement::VARDECL: {
					const auto& astTypeVarNode = statement->varDecl.typeVar;
					const auto& astInitialValueNode = statement->varDecl.value;
					
					const auto semValue = ConvertValue(context, astInitialValueNode);
					
					// Convert the AST type var.
					const bool isMemberVar = false;
					const auto semVar = ConvertInitialisedVar(context, isMemberVar, astTypeVarNode, semValue->type());
					assert(!semVar->isAny());
					
					// Cast the initialise value to the variable's type.
					// (The variable conversion above should have ensured
					// this will work.)
					const auto semInitialiseValue = ImplicitCast(semValue, semVar->constructType(), location);
					assert(!semInitialiseValue->type()->isVoid());
					
					// Add the variable to the SEM scope.
					const auto semScope = context.node().getSEMScope();
					semScope->localVariables().push_back(semVar);
					
					// Generate the initialise statement.
					return SEM::Statement::InitialiseStmt(semVar, semInitialiseValue);
				}
				case AST::Statement::RETURNVOID: {
					// Void return statement (i.e. return;)
					if (!getParentFunctionReturnType(context)->isVoid()) {
						throw ErrorException(makeString("Cannot return void in function '%s' with non-void return type at position %s.",
							lookupParentFunction(context).getSEMFunction()->name().toString().c_str(),
							location.toString().c_str()));
					}
					
					return SEM::Statement::ReturnVoid();
				}
				case AST::Statement::RETURN: {
					assert(statement->returnStmt.value.get() != nullptr);
					
					const auto semValue = ConvertValue(context, statement->returnStmt.value);
					
					// Cast the return value to the function's
					// specified return type.
					const auto castValue = ImplicitCast(semValue, getParentFunctionReturnType(context), location);
					
					return SEM::Statement::Return(castValue);
				}
				case AST::Statement::THROW: {
					const auto semValue = ConvertValue(context, statement->throwStmt.value);
					if (!semValue->type()->isObject() || !semValue->type()->getObjectType()->isException()) {
						throw ErrorException(makeString("Cannot throw non-exception value '%s' at position %s.",
								semValue->toString().c_str(), location.toString().c_str()));
					}
					return SEM::Statement::Throw(semValue);
				}
				case AST::Statement::BREAK: {
					// TODO: check that this is being used inside a loop or switch.
					return SEM::Statement::Break();
				}
				case AST::Statement::CONTINUE: {
					// TODO: check that this is being used inside a loop or switch.
					return SEM::Statement::Continue();
				}
				default:
					throw std::runtime_error("Unknown statement kind.");
			}
		}
		
		Debug::StatementInfo makeStatementInfo(const AST::Node<AST::Statement>& astStatementNode) {
			Debug::StatementInfo statementInfo;
			statementInfo.location = astStatementNode.location();
			return statementInfo;
		}
		
		SEM::Statement* ConvertStatement(Context& context, const AST::Node<AST::Statement>& astStatementNode) {
			const auto semStatement = ConvertStatementData(context, astStatementNode);
			context.debugModule().statementMap.insert(std::make_pair(semStatement, makeStatementInfo(astStatementNode)));
			return semStatement;
		}
		
	}
	
}


