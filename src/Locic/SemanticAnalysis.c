#include <stdio.h>
#include <Locic/SemanticAnalysis.h>
#include <Locic/SemanticContext.h>

SEM_Var * Locic_SEM_ConvertVar(Locic_SemanticContext * context, AST_Var * var){
	switch(var->type){
		case AST_VAR_LOCAL:
		{
			SEM_Var * semVar = Locic_SemanticContext_FindLocalVar(var->localVar.name);
			if(semVar != NULL){
				return semVar;
			}
			
			printf("Semantic Analysis Error: Local variable '%s' was not found\n", var->localVar.name);
			return NULL; 
		}
		case AST_VAR_THIS:
		{
			return NULL;
		}
		default:
			return NULL;
	}
}

SEM_Value * Locic_SEM_ConvertValue(Locic_SemanticContext * context, AST_Value * value){
	switch(value->type){
		case AST_VALUE_CONSTANT:
		{
			switch(value->constant.type){
				case AST_CONSTANT_BOOL:
					return SEM_MakeBoolConstant(value->constant.boolConstant);
				case AST_CONSTANT_INT:
					return SEM_MakeIntConstant(value->constant.intConstant);
				case AST_CONSTANT_FLOAT:
					return SEM_MakeFloatConstant(value->constant.floatConstant);
				default:
					return NULL;
			}
		}
		case AST_VALUE_VARACCESS:
		{
			SEM_Var * var = Locic_SEM_ConvertVar(context, value->varAccess.var);
			if(var == NULL){
				return NULL;
			}
			return SEM_MakeVarAccess(var);
		}
		case AST_VALUE_UNARY:
		{
			SEM_Value * operand = Locic_SEM_ConvertValue(context, value->unary.value);
			if(operand == NULL){
				return NULL;
			}
			
			switch(value->unary.type){
				case AST_UNARY_PLUS:
				{
					if(operand->type.typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum basicType = operand->type.basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT || basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_PLUS, operand, operand->type);
						}
					}
					printf("Semantic Analysis Error: Unary plus on non-numeric type\n");
					return NULL;
				}
				case AST_UNARY_MINUS:
				{
					if(operand->type.typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum basicType = operand->type.basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT || basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_MINUS, operand, operand->type);
						}
					}
					printf("Semantic Analysis Error: Unary minus on non-numeric type\n");
					return NULL;
				}
				case AST_UNARY_ADDRESSOF:
				{
					return SEM_MakeUnary(SEM_UNARY_ADDRESSOF, operand, SEM_MakePtrType(SEM_TYPE_MUTABLE, operand->type));
				}
				case AST_UNARY_DEREF:
				{
					if(operand->type.typeEnum != SEM_TYPE_PTR){
						return SEM_MakeUnary(SEM_UNARY_DEREF, operand, SEM_MakePtrType(SEM_TYPE_MUTABLE, operand->type));
					}
					
					printf("Semantic Analysis Error: Attempting to dereference non-pointer type\n");
					return NULL;
				}
				case AST_UNARY_NEGATE:
				{
					if(operand->type.typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum basicType = operand->type.basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT || basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_NEGATE, operand, operand->type);
						}
					}
					printf("Semantic Analysis Error: Negation on non-numeric type\n");
					return NULL;
				}
				default:
					return NULL;
			}
		}
		case AST_VALUE_BINARY:
		{
			SEM_Value * leftOperand, * rightOperand;
			leftOperand = Locic_SEM_ConvertValue(context, value->binary.left);
			rightOperand = Locic_SEM_ConvertValue(context, value->binary.right);
			if(leftOperand == NULL || rightOperand == NULL){
				return NULL;
			}
			
			switch(value->binary.type){
				case AST_BINARY_ADD:
				{
					if(leftOperand->type.typeEnum == SEM_TYPE_BASIC && rightOperand->type.typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type.basicType.typeEnum;
						rightBasicType = rightOperand->type.basicType.typeEnum;
						if((leftBasicType == SEM_TYPE_BASIC_INT || leftBasicType == SEM_TYPE_BASIC_FLOAT)
							&& leftBasicType == rightBasicType){
							return SEM_MakeBinary(SEM_BINARY_ADD, leftOperand, rightOperand, leftOperand->type);
						}
					}
					printf("Semantic Analysis Error: Addition between non-numeric or non-identical types\n");
					return NULL;
				}
				case AST_BINARY_SUBTRACT:
				{
					
					break;
				}
				case AST_BINARY_MULTIPLY:
				{
					
					break;
				}
				case AST_BINARY_DIVIDE:
				{
					
					break;
				}
				case AST_BINARY_ISEQUAL:
				{
					
					break;
				}
				case AST_BINARY_NOTEQUAL:
				{
					
					break;
				}
				case AST_BINARY_GREATEROREQUAL:
				{
					
					break;
				}
				case AST_BINARY_LESSOREQUAL:
				{
					
					break;
				}
				default:
					break;
			}
			return NULL;
		}
		case AST_VALUE_TERNARY:
		{
			
			return NULL;
		}
		case AST_VALUE_CONSTRUCT:
		{
			
			return NULL;
		}
		case AST_VALUE_MEMBERACCESS:
		{
			
			return NULL;
		}
		case AST_VALUE_METHODCALL:
		{
			
			return NULL;
		}
		default:
			return NULL;
	}
}

