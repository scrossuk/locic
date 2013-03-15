#include <cassert>
#include <cstdio>
#include <string>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertScope.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/ConvertValue.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool WillStatementReturn(SEM::Statement* statement) {
			switch(statement->kind()) {
				case SEM::Statement::VALUE: {
					return false;
				}
				case SEM::Statement::SCOPE: {
					return WillScopeReturn(statement->getScope());
				}
				case SEM::Statement::IF: {
					return WillScopeReturn(statement->getIfTrueScope()) &&
						   WillScopeReturn(statement->getIfFalseScope());
				}
				case SEM::Statement::WHILE: {
					return WillScopeReturn(statement->getWhileScope());
				}
				case SEM::Statement::ASSIGN: {
					return false;
				}
				case SEM::Statement::RETURN: {
					return true;
				}
				default: {
					printf("Internal Compiler Error: Unknown statement type in WillStatementReturn.\n");
					return false;
				}
			}
		}
		
		SEM::Statement* ConvertStatement(Context& context, AST::Statement* statement) {
			switch(statement->typeEnum) {
				case AST::Statement::VALUE: {
					SEM::Value* value = ConvertValue(context, statement->valueStmt.value);
					
					if(value != NULL) {
						return SEM::Statement::ValueStmt(value);
					}
					
					return NULL;
				}
				case AST::Statement::SCOPE: {
					return SEM::Statement::ScopeStmt(ConvertScope(context, statement->scopeStmt.scope));
				}
				case AST::Statement::IF: {
					SEM::Value* condition = ConvertValue(context, statement->ifStmt.condition);
					SEM::Scope* ifTrue = ConvertScope(context, statement->ifStmt.ifTrue);
					SEM::Scope* ifFalse = ConvertScope(context, statement->ifStmt.ifFalse);
					
					if(condition == NULL || ifTrue == NULL || ifFalse == NULL) {
						return NULL;
					}
					
					SEM::TypeInstance* boolType = context.getBuiltInType("bool");
					
					const std::vector<SEM::Type*> NO_TEMPLATE_ARGS;
					
					SEM::Value* boolValue = ImplicitCast(condition,
							SEM::Type::Object(SEM::Type::CONST, SEM::Type::RVALUE, boolType, NO_TEMPLATE_ARGS));
							
					return SEM::Statement::If(boolValue, ifTrue, ifFalse);
				}
				case AST::Statement::WHILE: {
					SEM::Value* condition = ConvertValue(context, statement->whileStmt.condition);
					SEM::Scope* whileTrue = ConvertScope(context, statement->whileStmt.whileTrue);
					
					if(condition == NULL || whileTrue == NULL) {
						return NULL;
					}
					
					SEM::TypeInstance* boolType = context.getBuiltInType("bool");
					
					const std::vector<SEM::Type*> NO_TEMPLATE_ARGS;
					
					SEM::Value* boolValue = ImplicitCast(condition,
							SEM::Type::Object(SEM::Type::CONST, SEM::Type::RVALUE, boolType, NO_TEMPLATE_ARGS));
							
					return SEM::Statement::While(boolValue, whileTrue);
				}
				case AST::Statement::VARDECL: {
					//AST::Type* typeAnnotation = statement->varDecl.type;
					//std::string varName = statement->varDecl.varName;
					AST::TypeVar* astTypeVar = statement->varDecl.typeVar;
					AST::Value* initialValue = statement->varDecl.value;
					
					SEM::Value* semValue = ConvertValue(context, initialValue);
					
					if(semValue == NULL) {
						return NULL;
					}
					
					SEM::Type* varType = NULL;
					
					if(astTypeVar->type == NULL) {
						// Auto keyword - use type of initial value.
						varType = semValue->type()->createLValueType();
					} else {
						// Using type annotation - verify that it is compatible with the type of the initial value.
						varType = ConvertType(context, astTypeVar->type, SEM::Type::LVALUE);
					}
					
					assert(varType != NULL);
					assert(varType->isLValue());
					
					if(varType->isVoid()) {
						printf("Semantic Analysis Error: Local variable cannot have void type.\n");
						return NULL;
					}
					
					SEM::Var* semVar = SEM::Var::Local(varType);
					
					const Node localVarNode = Node::Variable(astTypeVar, semVar);
					
					if(!context.node().tryAttach(astTypeVar->name, localVarNode)){
						// TODO: throw exception.
						assert(false && "Local variable name already exists.");
						return NULL;
					}
					
					return SEM::Statement::Assign(SEM::Value::VarValue(semVar),
							// The value being assigned must be an R-value.
							ImplicitCast(semValue, varType->createRValueType()));
				}
				case AST::Statement::ASSIGN: {
					SEM::Value* lValue = ConvertValue(context, statement->assignStmt.lValue);
					
					if(lValue == NULL) {
						return NULL;
					}
					
					if(!lValue->type()->isLValue()) {
						printf("Semantic Analysis Error: Cannot assign to r-value.\n");
						return NULL;
					}
					
					SEM::Value* rValue = ConvertValue(context, statement->assignStmt.rValue);
					
					if(rValue == NULL) {
						return NULL;
					}
					
					return SEM::Statement::Assign(lValue,
							ImplicitCast(rValue, lValue->type()->createRValueType()));
				}
				case AST::Statement::RETURN: {
					if(statement->returnStmt.value == NULL) {
						// Void return statement (i.e. return;)
						if(!context.getParentFunctionReturnType()->isVoid()) {
							printf("Semantic Analysis Error: Cannot return void in function with non-void return type.\n");
							return NULL;
						}
						
						return SEM::Statement::ReturnVoid();
					} else {
						SEM::Value* semValue = ConvertValue(context, statement->returnStmt.value);
						
						if(semValue == NULL) {
							return NULL;
						}
						
						SEM::Value* castValue = ImplicitCast(semValue, context.getParentFunctionReturnType());
						
						return SEM::Statement::Return(castValue);
					}
				}
				default:
					printf("Internal Compiler Error: Unknown statement type in 'ConvertStatement'.\n");
					return NULL;
			}
		}
		
	}
	
}


