#include <assert.h>

#include <set>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
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
					return WillScopeReturn(statement->getIfTrueScope()) &&
						   WillScopeReturn(statement->getIfFalseScope());
				}
				case SEM::Statement::SWITCH: {
					for (auto switchCase: statement->getSwitchCaseList()) {
						if (!WillScopeReturn(switchCase->scope())) {
							return false;
						}
					}
					return true;
				}
				case SEM::Statement::WHILE: {
					return WillScopeReturn(statement->getWhileScope());
				}
				case SEM::Statement::TRY: {
					// TODO: also consider catch blocks?
					return WillScopeReturn(statement->getTryScope());
				}
				case SEM::Statement::RETURN: {
					return true;
				}
				case SEM::Statement::THROW: {
					return true;
				}
				default: {
					throw std::runtime_error("Unknown statement kind.");
				}
			}
		}
		
		SEM::Statement* ConvertStatement(Context& context, AST::Node<AST::Statement> statement) {
			switch(statement->typeEnum) {
				case AST::Statement::VALUE: {
					const auto value = ConvertValue(context, statement->valueStmt.value);
					if (statement->valueStmt.hasVoidCast) {
						if (value->type()->isVoid()) {
							throw TodoException(makeString("Void explicitly ignored in expression '%s'.",
								value->toString().c_str()));
						}
						return SEM::Statement::ValueStmt(SEM::Value::Cast(SEM::Type::Void(), value));
					} else {
						if (!value->type()->isVoid()) {
							throw TodoException(makeString("Non-void value result ignored in expression '%s'.",
								value->toString().c_str()));
						}
						return SEM::Statement::ValueStmt(value);
					}
				}
				case AST::Statement::SCOPE: {
					return SEM::Statement::ScopeStmt(ConvertScope(context, statement->scopeStmt.scope));
				}
				case AST::Statement::IF: {
					const auto condition = ConvertValue(context, statement->ifStmt.condition);
					const auto ifTrue = ConvertScope(context, statement->ifStmt.ifTrue);
					const auto ifFalse = ConvertScope(context, statement->ifStmt.ifFalse);
					
					const auto boolType = getBuiltInType(context, "bool");
					
					const auto boolValue = ImplicitCast(condition,
							SEM::Type::Object(boolType, SEM::Type::NO_TEMPLATE_ARGS));
							
					return SEM::Statement::If(boolValue, ifTrue, ifFalse);
				}
				case AST::Statement::SWITCH: {
					auto value = ConvertValue(context, statement->switchStmt.value);
					
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
							throw TodoException(makeString("Duplicate switch case for type '%s'.",
								(*(insertResult.first))->refToString().c_str()));
						}
					}
					
					if (caseList.empty()) {
						throw TodoException("Switch statement must contain at least one case.");
					}
					
					// Check that all switch cases are based
					// on the same union datatype.
					const auto switchTypeInstance = (*(switchCaseTypeInstances.begin()))->parent();
					for (auto caseTypeInstance: switchCaseTypeInstances) {
						auto caseTypeInstanceParent = caseTypeInstance->parent();
						
						if (caseTypeInstanceParent == nullptr) {
							throw TodoException(makeString("Switch case type '%s' is not a member of a union datatype.",
								caseTypeInstance->refToString().c_str()));
						}
						
						if (caseTypeInstanceParent != switchTypeInstance) {
							throw TodoException(makeString("Switch case type '%s' does not share the same parent as type '%s'.",
								caseTypeInstance->refToString().c_str(),
								(*(switchCaseTypeInstances.begin()))->refToString().c_str()));
						}
					}
					
					// TODO: implement 'default' case.
					const bool hasDefaultCase = false;
					
					if (!hasDefaultCase) {
						// Check all cases are handled.
						for (auto variantTypeInstance: switchTypeInstance->variants()) {
							if (switchCaseTypeInstances.find(variantTypeInstance) == switchCaseTypeInstances.end()) {
								throw TodoException(makeString("Union datatype member '%s' not handled in switch.",
									variantTypeInstance->refToString().c_str()));
							}
						}
					}
					
					// Case value to switch type.
					const auto castValue = ImplicitCast(value,
							SEM::Type::Object(switchTypeInstance, SEM::Type::NO_TEMPLATE_ARGS));
					
					return SEM::Statement::Switch(castValue, caseList);
				}
				case AST::Statement::WHILE: {
					const auto condition = ConvertValue(context, statement->whileStmt.condition);
					const auto whileTrue = ConvertScope(context, statement->whileStmt.whileTrue);
					
					const auto boolType = getBuiltInType(context, "bool");
					
					const auto boolValue = ImplicitCast(condition,
							SEM::Type::Object(boolType, SEM::Type::NO_TEMPLATE_ARGS));
					
					return SEM::Statement::While(boolValue, whileTrue);
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
							throw TodoException("Try statement catch clauses may only contain named variables (no pattern matching).");
						}
						
						// Special case handling for catch variables,
						// since they don't use lvalues.
						const auto varType = ConvertType(context, astVar->namedVar.type);
						if (!varType->isException()) {
							throw TodoException(makeString("Type '%s' is not an exception type and therefore "
								"cannot be used in a catch clause.", varType->toString().c_str()));
						}
						
						const auto semVar = SEM::Var::Basic(varType, varType);
						attachVar(catchClauseContext, astVar->namedVar.name, astVar, semVar);
						
						semCatch->setVar(semVar);
						semCatch->setScope(ConvertScope(catchClauseContext, astCatch->scope));
						
						catchList.push_back(semCatch);
					}
					
					return SEM::Statement::Try(tryScope, catchList);
				}
				// TODO: replace code in parser with this.
				/*case AST::Statement::FOR: {
					AST::TypeVar* astTypeVar = statement->forStmt.typeVar;
					AST::Node<AST::Value> astRangeValue = statement->forStmt.rangeValue;
					
					SEM::Value* semRangeValue = ConvertValue(context, astRangeValue);
					
					// Create an anonymous variable to hold the range.
					SEM::Var* rangeVar = SEM::Var::Local(semRangeValue->type());
					
					// Start off by assigning the range value to the variable.
					SEM::Statement* setRangeVar = SEM::Statement::InitialiseStmt(rangeVar, semRangeValue);
					
					
					
					// Check whether a type annotation has been used.
					const bool autoType = (astTypeVar->type == NULL);
					
					// If type is 'auto', infer it from type value.
					SEM::Type* varType = autoType ? semValue->type() : ConvertType(context, astTypeVar->type);
					
					assert(varType != NULL);
					if (varType->isVoid()) {
						printf("Semantic Analysis Error: Local variable cannot have void type.\n");
						// TODO: throw exception.
						assert(false);
						return NULL;
					}
					
					return SEM::Statement::ScopeStmt(
				}*/
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
					const auto semInitialiseValue = ImplicitCast(semValue, semVar->constructType());
					assert(!semInitialiseValue->type()->isVoid());
					
					// Add the variable to the SEM scope.
					const auto semScope = context.node().getSEMScope();
					semScope->localVariables().push_back(semVar);
					
					// Generate the initialise statement.
					return SEM::Statement::InitialiseStmt(semVar, semInitialiseValue);
				}
				case AST::Statement::RETURN: {
					if (statement->returnStmt.value.get() == NULL) {
						// Void return statement (i.e. return;)
						if (!getParentFunctionReturnType(context)->isVoid()) {
							throw TodoException(makeString("Cannot return void in function '%s' with non-void return type.",
								lookupParentFunction(context).getSEMFunction()->name().toString().c_str()));
						}
						
						return SEM::Statement::ReturnVoid();
					} else {
						const auto semValue = ConvertValue(context, statement->returnStmt.value);
						
						// Cast the return value to the function's
						// specified return type.
						const auto castValue = ImplicitCast(semValue, getParentFunctionReturnType(context));
						
						return SEM::Statement::Return(castValue);
					}
				}
				case AST::Statement::THROW: {
					const auto semValue = ConvertValue(context, statement->throwStmt.value);
					if (!semValue->type()->isObject() || !semValue->type()->getObjectType()->isException()) {
						throw TodoException(makeString("Cannot throw non-exception value '%s'.",
								semValue->toString().c_str()));
					}
					return SEM::Statement::Throw(semValue);
				}
				default:
					throw std::runtime_error("Unknown statement kind.");
			}
		}
		
	}
	
}


