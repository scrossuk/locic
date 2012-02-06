#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertType.h>
#include <Locic/SemanticAnalysis/ConvertValue.h>

SEM_Value * Locic_SemanticAnalysis_ConvertValue(Locic_SemanticContext * context, AST_Value * value){
	if(value == NULL){
		printf("Internal compiler error: Cannot convert NULL AST_Value.\n");
		return NULL;
	}

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
					printf("Internal Compiler Error: Unknown constant type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_VAR:
		{
			AST_Var * synVar = value->varValue.var;
			switch(synVar->type){
				case AST_VAR_LOCAL:
				{
					SEM_Var * semVar = Locic_SemanticContext_FindLocalVar(context, synVar->localVar.name);
					if(semVar != NULL){
						return SEM_MakeVarValue(semVar);
					}
					
					// Not a variable - try looking for functions.
					SEM_FunctionDecl * decl = Locic_StringMap_Find(context->functionDeclarations, synVar->localVar.name);
					
					if(decl != NULL){
						return SEM_MakeFunctionRef(decl, decl->type);
					}
					
					printf("Semantic Analysis Error: Local variable '%s' was not found\n", synVar->localVar.name);
					return NULL; 
				}
				case AST_VAR_THIS:
				{
					printf("Semantic Analysis Error: Member variables not implemented.\n");
					return NULL;
				}
				default:
					printf("Internal Compiler Error: Unknown AST_Var type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_UNARY:
		{
			SEM_Value * operand = Locic_SemanticAnalysis_ConvertValue(context, value->unary.value);
			if(operand == NULL){
				return NULL;
			}
			
			switch(value->unary.type){
				case AST_UNARY_PLUS:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						SEM_BasicTypeEnum basicType = typeCopy->basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT){
							return SEM_MakeUnary(SEM_UNARY_PLUS, SEM_OP_INT, operand, typeCopy);
						}else if(basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_PLUS, SEM_OP_FLOAT, operand, typeCopy);
						}
					}
					printf("Semantic Analysis Error: Unary plus on non-numeric type.\n");
					return NULL;
				}
				case AST_UNARY_MINUS:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						SEM_BasicTypeEnum basicType = typeCopy->basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT){
							return SEM_MakeUnary(SEM_UNARY_MINUS, SEM_OP_INT, operand, typeCopy);
						}else if(basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_MINUS, SEM_OP_FLOAT, operand, typeCopy);
						}
					}
					printf("Semantic Analysis Error: Unary minus on non-numeric type.\n");
					return NULL;
				}
				case AST_UNARY_ADDRESSOF:
				{
					if(operand->type->isLValue == SEM_TYPE_LVALUE){
						return SEM_MakeUnary(SEM_UNARY_ADDRESSOF, SEM_OP_PTR, operand, SEM_MakePtrType(SEM_TYPE_MUTABLE, SEM_TYPE_RVALUE, operand->type));
					}
					
					printf("Semantic Analysis Error: Attempting to take address of R-value.\n");
					return NULL;
				}
				case AST_UNARY_DEREF:
				{
					if(operand->type->typeEnum == SEM_TYPE_PTR){
						return SEM_MakeUnary(SEM_UNARY_DEREF, SEM_OP_PTR, operand, operand->type->ptrType.ptrType);
					}
					
					printf("Semantic Analysis Error: Attempting to dereference non-pointer type.\n");
					return NULL;
				}
				case AST_UNARY_NOT:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						if(typeCopy->basicType.typeEnum == SEM_TYPE_BASIC_BOOL){
							return SEM_MakeUnary(SEM_UNARY_NOT, SEM_OP_BOOL, operand, typeCopy);
						}
					}
					
					printf("Semantic Analysis Error: Unary NOT on non-bool type.\n");
					return NULL;
				}
				default:
					printf("Internal Compiler Error: Unknown unary value type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_BINARY:
		{
			SEM_Value * leftOperand, * rightOperand;
			leftOperand = Locic_SemanticAnalysis_ConvertValue(context, value->binary.left);
			rightOperand = Locic_SemanticAnalysis_ConvertValue(context, value->binary.right);
			if(leftOperand == NULL || rightOperand == NULL){
				return NULL;
			}
			
			switch(value->binary.type){
				case AST_BINARY_ADD:
				{
					if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type->basicType.typeEnum;
						rightBasicType = rightOperand->type->basicType.typeEnum;
						
						if(leftBasicType == rightBasicType){
							SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
							typeCopy->isLValue = SEM_TYPE_RVALUE;
							
							if(leftBasicType == SEM_TYPE_BASIC_INT){
								return SEM_MakeBinary(SEM_BINARY_ADD, SEM_OP_INT, leftOperand, rightOperand, typeCopy);
							}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
								return SEM_MakeBinary(SEM_BINARY_ADD, SEM_OP_FLOAT, leftOperand, rightOperand, typeCopy);
							}
						}
					}
					printf("Semantic Analysis Error: Addition between non-numeric or non-identical types.\n");
					return NULL;
				}
				case AST_BINARY_SUBTRACT:
				{
					if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type->basicType.typeEnum;
						rightBasicType = rightOperand->type->basicType.typeEnum;
						
						if(leftBasicType == rightBasicType){
							SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
							typeCopy->isLValue = SEM_TYPE_RVALUE;
							
							if(leftBasicType == SEM_TYPE_BASIC_INT){
								return SEM_MakeBinary(SEM_BINARY_SUBTRACT, SEM_OP_INT, leftOperand, rightOperand, typeCopy);
							}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
								return SEM_MakeBinary(SEM_BINARY_SUBTRACT, SEM_OP_FLOAT, leftOperand, rightOperand, typeCopy);
							}
						}
					}
					printf("Semantic Analysis Error: Subtraction between non-numeric or non-identical types.\n");
					return NULL;
				}
				case AST_BINARY_MULTIPLY:
				{
					if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type->basicType.typeEnum;
						rightBasicType = rightOperand->type->basicType.typeEnum;
						
						if(leftBasicType == rightBasicType){
							SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
							typeCopy->isLValue = SEM_TYPE_RVALUE;
							
							if(leftBasicType == SEM_TYPE_BASIC_INT){
								return SEM_MakeBinary(SEM_BINARY_MULTIPLY, SEM_OP_INT, leftOperand, rightOperand, typeCopy);
							}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
								return SEM_MakeBinary(SEM_BINARY_MULTIPLY, SEM_OP_FLOAT, leftOperand, rightOperand, typeCopy);
							}
						}
					}
					printf("Semantic Analysis Error: Multiplication between non-numeric or non-identical types.\n");
					return NULL;
				}
				case AST_BINARY_DIVIDE:
				{
					if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_BasicTypeEnum leftBasicType, rightBasicType;
						leftBasicType = leftOperand->type->basicType.typeEnum;
						rightBasicType = rightOperand->type->basicType.typeEnum;
						
						if(leftBasicType == rightBasicType){
							SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
							typeCopy->isLValue = SEM_TYPE_RVALUE;
							
							if(leftBasicType == SEM_TYPE_BASIC_INT){
								return SEM_MakeBinary(SEM_BINARY_DIVIDE, SEM_OP_INT, leftOperand, rightOperand, typeCopy);
							}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
								return SEM_MakeBinary(SEM_BINARY_DIVIDE, SEM_OP_FLOAT, leftOperand, rightOperand, typeCopy);
							}
						}
					}
					printf("Semantic Analysis Error: Division between non-numeric or non-identical types.\n");
					return NULL;
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
					printf("Internal Compiler Error: Unknown binary value type enum.\n");
					return NULL;
			}
			printf("Internal Compiler Error: Unimplemented binary operator.\n");
			return NULL;
		}
		case AST_VALUE_TERNARY:
		{
			printf("Internal Compiler Error: Unimplemented ternary operator.\n");
			return NULL;
		}
		case AST_VALUE_CAST:
		{
			AST_Cast * cast = &(value->cast);
			SEM_Type * type = Locic_SemanticAnalysis_ConvertType(context, cast->type, SEM_TYPE_RVALUE);
			SEM_Value * val = Locic_SemanticAnalysis_ConvertValue(context, cast->value);
			
			if(Locic_SemanticAnalysis_CanDoExplicitCast(context, val->type, type) != 1){
				printf("Semantic Analysis Error: Can't perform explicit cast.\n");
				return NULL;
			}
			
			return SEM_MakeCast(type, val);
		}
		case AST_VALUE_CONSTRUCT:
		{
			printf("Internal Compiler Error: Unimplemented constructor call.\n");
			return NULL;
		}
		case AST_VALUE_MEMBERACCESS:
		{
			printf("Internal Compiler Error: Unimplemented member access.\n");
			return NULL;
		}
		case AST_VALUE_FUNCTIONCALL:
		{
			SEM_Value * functionValue = Locic_SemanticAnalysis_ConvertValue(context, value->functionCall.functionValue);
			
			if(functionValue == NULL){
				return NULL;
			}
			
			if(functionValue->type->typeEnum != SEM_TYPE_FUNC){
				printf("Semantic Analysis Error: Can't call non-function type.\n");
				return NULL;
			}
			
			Locic_List * typeList = functionValue->type->funcType.parameterTypes;
			Locic_List * synValueList = value->functionCall.parameters;
			
			if(Locic_List_Size(typeList) != Locic_List_Size(synValueList)){
				printf("Semantic Analysis Error: Function called with %lu number of parameters; expected %lu.\n", Locic_List_Size(synValueList), Locic_List_Size(typeList));
				return NULL;
			}
			
			Locic_List * semValueList = Locic_List_Alloc();
			
			Locic_ListElement * typeIt = Locic_List_Begin(typeList);
			Locic_ListElement * valueIt = Locic_List_Begin(synValueList);
			
			while(valueIt != Locic_List_End(synValueList)){
				SEM_Value * param = Locic_SemanticAnalysis_ConvertValue(context, valueIt->data);
				
				if(param == NULL){
					return NULL;
				}
			
				if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, param->type, typeIt->data)){
					printf("Semantic Analysis Error: Cannot convert parameter value to type expected by function.\n");
					return NULL;
				}
				
				Locic_List_Append(semValueList, param);
				
				typeIt = typeIt->next;
				valueIt = valueIt->next;
			}
			
			return SEM_MakeFunctionCall(functionValue, semValueList, functionValue->type->funcType.returnType);
		}
		default:
			printf("Internal Compiler Error: Unknown AST_Value type enum.\n");
			return NULL;
	}
}

