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

SEM_Type * SEM_MakeClassType(SEM_TypeIsMutable isMutable, SEM_ClassDecl * classDecl){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_CLASS, isMutable);
	(type->classType).classDecl = classDecl;
	return type;
}

SEM_Type * SEM_MakePtrType(SEM_TypeIsMutable isMutable, SEM_Type * ptrType){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_PTR, isMutable);
	(type->ptrType).ptrType = ptrType;
	return type;
}


