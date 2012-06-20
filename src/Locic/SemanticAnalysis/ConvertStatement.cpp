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
			switch(statement->typeEnum) {
				case SEM::Statement::VALUE: {
					return false;
				}
				case SEM::Statement::SCOPE: {
					return WillScopeReturn(statement->scopeStmt.scope);
				}
				case SEM::Statement::IF: {
					return WillScopeReturn(statement->ifStmt.ifTrue) &&
					       WillScopeReturn(statement->ifStmt.ifFalse);
				}
				case SEM::Statement::WHILE: {
					return WillScopeReturn(statement->whileStmt.whileTrue);
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
		
		SEM::Statement* ConvertStatement(LocalContext& context, AST::Statement* statement) {
			switch(statement->typeEnum) {
				case AST::Statement::VALUE: {
					SEM::Value* value = ConvertValue(context, statement->valueStmt.value);
						
					if(value != NULL) {
						return SEM::Statement::ValueStmt(value);
					}
						
					return NULL;
				}
				case AST::Statement::SCOPE: {
					SEM::Scope* scope = ConvertScope(context, statement->scopeStmt.scope);
						
					if(scope == NULL) {
						return NULL;
					}
					
					return SEM::Statement::ScopeStmt(scope);
				}
				case AST::Statement::IF: {
					SEM::Value* condition = ConvertValue(context, statement->ifStmt.condition);
					SEM::Scope* ifTrue = ConvertScope(context, statement->ifStmt.ifTrue);
					SEM::Scope* ifFalse = ConvertScope(context, statement->ifStmt.ifFalse);
					
					if(condition == NULL || ifTrue == NULL || ifFalse == NULL) {
						return NULL;
					}
					
					SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
					
					SEM::Value* boolValue = CastValueToType(context, condition, boolType);
					
					if(boolValue == NULL) {
						printf("Semantic Analysis Error: Cannot cast or copy conditionition type '%s' to bool type in IF statement.\n",
						       condition->type->toString().c_str());
						return NULL;
					}
					
					return SEM::Statement::If(boolValue, ifTrue, ifFalse);
				}
				case AST::Statement::WHILE: {
					SEM::Value* condition = ConvertValue(context, statement->whileStmt.condition);
					SEM::Scope* whileTrue = ConvertScope(context, statement->whileStmt.whileTrue);
					
					if(condition == NULL || whileTrue == NULL) {
						return NULL;
					}
					
					SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
					
					SEM::Value* boolValue = CastValueToType(context, condition, boolType);
					
					if(boolValue == NULL) {
						printf("Semantic Analysis Error: Cannot cast or copy conditionition type '%s' to bool type in WHILE statement.\n",
						       condition->type->toString().c_str());
						return NULL;
					}
					
					return SEM::Statement::While(boolValue, whileTrue);
				}
				case AST::Statement::VARDECL: {
					AST::Type* typeAnnotation = statement->varDecl.type;
					std::string varName = statement->varDecl.varName;
					AST::Value* initialValue = statement->varDecl.value;
					
					SEM::Value* semValue = ConvertValue(context, initialValue);
						
					if(semValue == NULL) {
						return NULL;
					}
					
					const bool canCopyValue = CanDoImplicitCopy(context, semValue->type);
					
					SEM::Type* type;
					
					if(typeAnnotation == NULL) {
						// Auto keyword - use type of initial value.
						type = new SEM::Type(*(semValue->type));
						type->isMutable = true;
						type->isLValue = true;
						
						// If the value is const, and it can't be copied,
						// the variable must also be const.
						if(!semValue->type->isMutable && !canCopyValue) {
							type->isMutable = false;
						}
					} else {
						// Using type annotation - verify that it is compatible with the type of the initial value.
						type = ConvertType(context, typeAnnotation, SEM::Type::LVALUE);
							
						if(type == NULL) {
							return NULL;
						}
					}
					
					if(type->typeEnum == SEM::Type::VOID) {
						printf("Semantic Analysis Error: Local variable cannot have void type.\n");
						return NULL;
					}
					
					SEM::Var* semVar = context.defineLocalVar(varName, type);
						
					if(semVar == NULL) {
						printf("Semantic Analysis Error: Local variable name already exists.\n");
						return NULL;
					}
					
					SEM::Value* castValue = CastValueToType(context, semValue, type);
					
					if(castValue == NULL) {
						// 'Auto' type annotations shouldn't cause problems like this.
						assert(typeAnnotation != NULL);
						
						printf("Semantic Analysis Error: Cannot cast or copy value type '%s' to annotated variable type '%s' in declaration.\n",
						       semValue->type->toString().c_str(),
						       type->toString().c_str());
						return NULL;
					}
					
					return SEM::Statement::Assign(SEM::Value::VarValue(semVar), castValue);
				}
				case AST::Statement::ASSIGN: {
					SEM::Value* lValue = ConvertValue(context, statement->assignStmt.lValue);
						
					if(lValue == NULL) {
						return NULL;
					}
					
					if(!lValue->type->isMutable) {
						printf("Semantic Analysis Error: Cannot assign to const value.\n");
						return NULL;
					}
					
					if(!lValue->type->isLValue) {
						printf("Semantic Analysis Error: Cannot assign to r-value.\n");
						return NULL;
					}
					
					SEM::Value* rValue = ConvertValue(context, statement->assignStmt.rValue);
						
					if(rValue == NULL) {
						return NULL;
					}
					
					SEM::Value* castRValue = CastValueToType(context, rValue, lValue->type);
						
					if(castRValue == NULL) {
						printf("Semantic Analysis Error: Cannot cast or copy rvalue type '%s' to lvalue type '%s' in assignment.\n",
						       rValue->type->toString().c_str(),
						       lValue->type->toString().c_str());
						return NULL;
					}
					
					return SEM::Statement::Assign(lValue, castRValue);
				}
				case AST::Statement::RETURN: {
					if(statement->returnStmt.value == NULL) {
						// Void return statement (i.e. return;)
						if(!context.getReturnType()->isVoid()) {
							printf("Semantic Analysis Error: Cannot return void in function with non-void return type.\n");
							return NULL;
						}
							
						return SEM::Statement::ReturnVoid();
					} else {
						SEM::Value* semValue = ConvertValue(context, statement->returnStmt.value);
							
						if(semValue == NULL) {
							return NULL;
						}
						
						SEM::Value* castValue = CastValueToType(context, semValue, context.getReturnType());
						
						if(castValue == NULL) {
							printf("Semantic Analysis Error: Cannot cast or copy value type '%s' to function return type '%s' in return statement.\n",
							       semValue->type->toString().c_str(),
							       context.getReturnType()->toString().c_str());
							return NULL;
						}
						
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


