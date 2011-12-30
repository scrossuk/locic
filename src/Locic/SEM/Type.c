#include <stdlib.h>
#include <Locic/SEM/Type.h>

inline SEM_Type * SEM_AllocateType(SEM_TypeEnum typeEnum, SEM_TypeIsMutable isMutable){
	SEM_Type * type = malloc(sizeof(SEM_Type));
	type->typeEnum = typeEnum;
	type->isMutable = isMutable;
	return type;
}

SEM_Type * SEM_MakeBasicType(SEM_TypeIsMutable isMutable, SEM_BasicTypeEnum typeEnum){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_BASIC, isMutable);
	(type->basicType).typeEnum = typeEnum;
	return type;
}

SEM_Type * SEM_MakeNamedType(SEM_TypeIsMutable isMutable, char * name){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_NAMED, isMutable);
	(type->namedType).name = name;
	return type;
}

SEM_Type * SEM_MakePtrType(SEM_TypeIsMutable isMutable, SEM_Type * ptrType){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_PTR, isMutable);
	(type->ptrType).ptrType = ptrType;
	return type;
}

int SEM_CompareTypes(SEM_Type * firstType, SEM_Type * secondType){
	if(firstType->typeEnum != secondType->typeEnum){
		return 0;
	}
	
	// TODO
	return 1;
}

int SEM_TypeIsNumeric(SEM_Type * type){
	
}


