#include <stdlib.h>
#include <Locic/List.h>
#include <Locic/SEM/Type.h>

inline SEM_Type * SEM_AllocateType(SEM_TypeEnum typeEnum, SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue){
	SEM_Type * type = malloc(sizeof(SEM_Type));
	type->typeEnum = typeEnum;
	type->isMutable = isMutable;
	type->isLValue = isLValue;
	return type;
}

SEM_Type * SEM_MakeBasicType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_BasicTypeEnum typeEnum){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_BASIC, isMutable, isLValue);
	(type->basicType).typeEnum = typeEnum;
	return type;
}

SEM_Type * SEM_MakeClassType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_ClassDecl * classDecl){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_CLASS, isMutable, isLValue);
	(type->classType).classDecl = classDecl;
	return type;
}

SEM_Type * SEM_MakePtrType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_Type * ptrType){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_PTR, isMutable, isLValue);
	(type->ptrType).ptrType = ptrType;
	return type;
}

SEM_Type * SEM_MakeFuncType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_Type * returnType, Locic_List * parameterTypes){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_FUNC, isMutable, isLValue);
	(type->funcType).returnType = returnType;
	(type->funcType).parameterTypes = parameterTypes;
	return type;
}

SEM_Type * SEM_CopyType(SEM_Type * type){
	SEM_Type * newType = malloc(sizeof(SEM_Type));
	newType->typeEnum = type->typeEnum;
	newType->isMutable = type->isMutable;
	newType->isLValue = type->isLValue;
	
	switch(type->typeEnum){
		case SEM_TYPE_BASIC:
			newType->basicType = type->basicType;
			break;
		case SEM_TYPE_CLASS:
			newType->classType = type->classType;
			break;
		case SEM_TYPE_PTR:
			newType->ptrType = type->ptrType;
			break;
		case SEM_TYPE_FUNC:
			newType->funcType = type->funcType;
			break;
		default:
			break;
	}
	return newType;
}

int SEM_IsVoidType(SEM_Type * type){
	if(type == NULL) return 0;
	if(type->typeEnum != SEM_TYPE_BASIC) return 0;
	if(type->basicType.typeEnum != SEM_TYPE_BASIC_VOID) return 0;
	return 1;
}



