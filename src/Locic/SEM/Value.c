#include <stdlib.h>
#include <Locic/List.h>
#include <Locic/SEM/Function.h>
#include <Locic/SEM/Type.h>
#include <Locic/SEM/Value.h>
#include <Locic/SEM/Var.h>

inline SEM_Value * SEM_AllocateValue(SEM_ValueType type){
	SEM_Value * value = malloc(sizeof(SEM_Value));
	value->valueType = type;
	return value;
}

/* SEM_Constant */

SEM_Value * SEM_MakeBoolConstant(int val){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CONSTANT);
	SEM_Constant * constant = &(value->constant);
	constant->type = SEM_CONSTANT_BOOL;
	constant->boolConstant = val;
	value->type = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
	return value;
}

SEM_Value * SEM_MakeIntConstant(int val){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CONSTANT);
	SEM_Constant * constant = &(value->constant);
	constant->type = SEM_CONSTANT_INT;
	constant->intConstant = val;
	value->type = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_INT);
	return value;
}

SEM_Value * SEM_MakeFloatConstant(float val){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CONSTANT);
	SEM_Constant * constant = &(value->constant);
	constant->type = SEM_CONSTANT_FLOAT;
	constant->floatConstant = val;
	value->type = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_FLOAT);
	return value;
}

/* SEM_CopyValue */

SEM_Value * SEM_MakeCopyValue(SEM_Value * operand){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_COPY);
	(value->copyValue).value = operand;
	
	SEM_Type * type = SEM_CopyType(operand->type);
	type->isLValue = SEM_TYPE_RVALUE;
	value->type = type;
	return value;
}

/* SEM_VarValue */

SEM_Value * SEM_MakeVarValue(SEM_Var * var){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_VAR);
	(value->varValue).var = var;
	value->type = var->type;
	return value;
}

/* SEM_Unary */

SEM_Value * SEM_MakeUnary(SEM_UnaryType unaryType, SEM_OpType opType, SEM_Value * operand, SEM_Type * type){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_UNARY);
	SEM_Unary * unary = &(value->unary);
	unary->type = unaryType;
	unary->opType = opType;
	unary->value = operand;
	value->type = type;
	return value;
}

/* SEM_Binary */

SEM_Value * SEM_MakeBinary(SEM_BinaryType binaryType, SEM_OpType opType, SEM_Value * left, SEM_Value * right, SEM_Type * type){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_BINARY);
	SEM_Binary * binary = &(value->binary);
	binary->type = binaryType;
	binary->opType = opType;
	binary->left = left;
	binary->right = right;
	value->type = type;
	return value;
}

/* SEM_Ternary */

SEM_Value * SEM_MakeTernary(SEM_Value * cond, SEM_Value * ifTrue, SEM_Value * ifFalse, SEM_Type * type){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_TERNARY);
	SEM_Ternary * ternary = &(value->ternary);
	ternary->condition = cond;
	ternary->ifTrue = ifTrue;
	ternary->ifFalse = ifFalse;
	value->type = type;
	return value;
}

/* SEM_Cast */

SEM_Value * SEM_MakeCast(SEM_Type * type, SEM_Value * val){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CAST);
	value->type = type;
	SEM_Cast * cast = &(value->cast);
	cast->value = val;
	return value;
}

/* SEM_Construct */

SEM_Value * SEM_MakeConstruct(SEM_ClassDecl * classDecl, size_t constructorId, Locic_List * parameters, SEM_Type * type){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_CONSTRUCT);
	SEM_Construct * construct = &(value->construct);
	construct->classDecl = classDecl;
	construct->constructorId = constructorId;
	construct->parameters = parameters;
	value->type = type;
	return value;
}

/* SEM_MemberAccess */

SEM_Value * SEM_MakeMemberAccess(SEM_Value * object, size_t memberId){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_MEMBERACCESS);
	SEM_MemberAccess * memberAccess = &(value->memberAccess);
	memberAccess->object = object;
	memberAccess->memberId = memberId;
	return value;
}

/* SEM_FunctionCall */

SEM_Value * SEM_MakeFunctionCall(SEM_Value * functionValue, Locic_List * parameters, SEM_Type * type){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_FUNCTIONCALL);
	SEM_FunctionCall * functionCall = &(value->functionCall);
	functionCall->functionValue = functionValue;
	functionCall->parameters = parameters;
	value->type = type;
	return value;
}

/* SEM_FunctionRef */

SEM_Value * SEM_MakeFunctionRef(SEM_FunctionDecl * decl, SEM_Type * type){
	SEM_Value * value = SEM_AllocateValue(SEM_VALUE_FUNCTIONREF);
	value->functionRef.functionDecl = decl;
	value->type = type;
	return value;
}




