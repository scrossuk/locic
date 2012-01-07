#include <stdlib.h>
#include <Locic/AST/Type.h>

inline AST_Type * AST_AllocateType(AST_TypeEnum typeEnum, AST_TypeIsMutable isMutable){
	AST_Type * type = malloc(sizeof(AST_Type));
	type->typeEnum = typeEnum;
	type->isMutable = isMutable;
	return type;
}

AST_Type * AST_MakeBasicType(AST_TypeIsMutable isMutable, AST_BasicTypeEnum typeEnum){
	AST_Type * type = AST_AllocateType(AST_TYPE_BASIC, isMutable);
	(type->basicType).typeEnum = typeEnum;
	return type;
}

AST_Type * AST_MakeNamedType(AST_TypeIsMutable isMutable, char * name){
	AST_Type * type = AST_AllocateType(AST_TYPE_NAMED, isMutable);
	(type->namedType).name = name;
	return type;
}

AST_Type * AST_MakePtrType(AST_TypeIsMutable isMutable, AST_Type * ptrType){
	AST_Type * type = AST_AllocateType(AST_TYPE_PTR, isMutable);
	(type->ptrType).ptrType = ptrType;
	return type;
}

AST_Type * AST_MakeFuncType(AST_TypeIsMutable isMutable, AST_Type * returnType, Locic_List * parameterTypes){
	AST_Type * type = AST_AllocateType(AST_TYPE_FUNC, isMutable);
	(type->funcType).returnType = returnType;
	(type->funcType).parameterTypes = parameterTypes;
	return type;
}

