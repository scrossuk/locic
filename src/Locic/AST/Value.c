#include <stdlib.h>
#include <Locic/List.h>
#include <Locic/AST/Value.h>
#include <Locic/AST/Var.h>

inline AST_Value * AST_AllocateValue(AST_ValueType type){
	AST_Value * value = malloc(sizeof(AST_Value));
	value->type = type;
	return value;
}

/* AST_Constant */

AST_Value * AST_MakeBoolConstant(int val){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CONSTANT);
	AST_Constant * constant = &(value->constant);
	constant->type = AST_CONSTANT_BOOL;
	constant->boolConstant = val;
	return value;
}

AST_Value * AST_MakeIntConstant(int val){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CONSTANT);
	AST_Constant * constant = &(value->constant);
	constant->type = AST_CONSTANT_INT;
	constant->intConstant = val;
	return value;
}

AST_Value * AST_MakeFloatConstant(float val){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CONSTANT);
	AST_Constant * constant = &(value->constant);
	constant->type = AST_CONSTANT_FLOAT;
	constant->floatConstant = val;
	return value;
}

AST_Value * AST_MakeNullConstant(){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CONSTANT);
	AST_Constant * constant = &(value->constant);
	constant->type = AST_CONSTANT_NULL;
	return value;
}

/* AST_Var */

AST_Value * AST_MakeVarValue(AST_Var * var){
	AST_Value * value = AST_AllocateValue(AST_VALUE_VAR);
	(value->varValue).var = var;
	return value;
}

/* AST_Unary */

AST_Value * AST_MakeUnary(AST_UnaryType type, AST_Value * operand){
	AST_Value * value = AST_AllocateValue(AST_VALUE_UNARY);
	AST_Unary * unary = &(value->unary);
	unary->type = type;
	unary->value = operand;
	return value;
}

/* AST_Binary */

AST_Value * AST_MakeBinary(AST_BinaryType type, AST_Value * left, AST_Value * right){
	AST_Value * value = AST_AllocateValue(AST_VALUE_BINARY);
	AST_Binary * binary = &(value->binary);
	binary->type = type;
	binary->left = left;
	binary->right = right;
	return value;
}

/* AST_Ternary */

AST_Value * AST_MakeTernary(AST_Value * cond, AST_Value * ifTrue, AST_Value * ifFalse){
	AST_Value * value = AST_AllocateValue(AST_VALUE_TERNARY);
	AST_Ternary * ternary = &(value->ternary);
	ternary->condition = cond;
	ternary->ifTrue = ifTrue;
	ternary->ifFalse = ifFalse;
	return value;
}

/* AST_Cast */

AST_Value * AST_MakeCast(AST_Type * type, AST_Value * val){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CAST);
	AST_Cast * cast = &(value->cast);
	cast->type = type;
	cast->value = val;
	return value;
}

/* AST_Construct */

AST_Value * AST_MakeConstruct(char * typeName, char * constructorName, Locic_List * parameters){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CONSTRUCT);
	AST_Construct * construct = &(value->construct);
	construct->typeName = typeName;
	construct->constructorName = constructorName;
	construct->parameters = parameters;
	return value;
}

/* AST_MemberAccess */

AST_Value * AST_MakeMemberAccess(AST_Value * object, char * memberName){
	AST_Value * value = AST_AllocateValue(AST_VALUE_MEMBERACCESS);
	AST_MemberAccess * memberAccess = &(value->memberAccess);
	memberAccess->object = object;
	memberAccess->memberName = memberName;
	return value;
}

/* AST_FunctionCall */

AST_Value * AST_MakeFunctionCall(AST_Value * functionValue, Locic_List * parameters){
	AST_Value * value = AST_AllocateValue(AST_VALUE_FUNCTIONCALL);
	AST_FunctionCall * functionCall = &(value->functionCall);
	functionCall->functionValue = functionValue;
	functionCall->parameters = parameters;
	return value;
}




