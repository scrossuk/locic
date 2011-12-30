#include <stdlib.h>
#include <Locic/List.h>
#include <Locic/SEM/Value.h>
#include <Locic/SEM/Var.h>

inline SEM_Value * SEM_AllocateValue(SEM_ValueType type){
	SEM_Value * value = malloc(sizeof(SEM_Value));
	value->type = type;
	return value;
}

/* SEM_Constant */

SEM_Value * SEM_MakeBoolConstant(int val){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CONSTANT);
	SEM_Constant * constant = &(value->constant);
	constant->type = SEM_CONSTANT_BOOL;
	constant->boolConstant = val;
	return value;
}

SEM_Value * SEM_MakeIntConstant(int val){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CONSTANT);
	SEM_Constant * constant = &(value->constant);
	constant->type = SEM_CONSTANT_INT;
	constant->intConstant = val;
	return value;
}

SEM_Value * SEM_MakeFloatConstant(float val){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CONSTANT);
	SEM_Constant * constant = &(value->constant);
	constant->type = SEM_CONSTANT_FLOAT;
	constant->floatConstant = val;
	return value;
}

/* SEM_Var */

SEM_Value * SEM_MakeVarAccess(SEM_Var * var){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_VARACCESS);
	(value->varAccess).var = var;
	return value;
}

/* SEM_Unary */

SEM_Value * SEM_MakeUnary(SEM_UnaryType type, SEM_Value * operand){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_UNARY);
	SEM_Unary * unary = &(value->unary);
	unary->type = type;
	unary->value = operand;
	return value;
}

/* SEM_Binary */

SEM_Value * SEM_MakeBinary(SEM_BinaryType type, SEM_Value * left, SEM_Value * right){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_BINARY);
	SEM_Binary * binary = &(value->binary);
	binary->type = type;
	binary->left = left;
	binary->right = right;
	return value;
}

/* SEM_Ternary */

SEM_Value * SEM_MakeTernary(SEM_Value * cond, SEM_Value * ifTrue, SEM_Value * ifFalse){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_TERNARY);
	SEM_Ternary * ternary = &(value->ternary);
	ternary->condition = cond;
	ternary->ifTrue = ifTrue;
	ternary->ifFalse = ifFalse;
	return value;
}

/* SEM_Construct */

SEM_Value * SEM_MakeConstruct(char * typeName, char * constructorName, Locic_List * parameters){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CONSTRUCT);
	SEM_Construct * construct = &(value->construct);
	construct->typeName = typeName;
	construct->constructorName = constructorName;
	construct->parameters = parameters;
	return value;
}

/* SEM_MemberAccess */

SEM_Value * SEM_MakeMemberAccess(SEM_Value * object, char * memberName){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_MEMBERACCESS);
	SEM_MemberAccess * memberAccess = &(value->memberAccess);
	memberAccess->object = object;
	memberAccess->memberName = memberName;
	return value;
}

/* SEM_MethodCall */

SEM_Value * SEM_MakeMethodCall(SEM_Value * object, char * methodName, Locic_List * parameters){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_METHODCALL);
	SEM_MethodCall * methodCall = &(value->methodCall);
	methodCall->object = object;
	methodCall->methodName = methodName;
	methodCall->parameters = parameters;
	return value;
}




