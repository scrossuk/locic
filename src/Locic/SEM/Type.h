#ifndef LOCIC_SEM_TYPE_H
#define LOCIC_SEM_TYPE_H

typedef enum SEM_TypeEnum{
	SEM_TYPE_BASIC,
	SEM_TYPE_CLASS,
	SEM_TYPE_PTR
} SEM_TypeEnum;

typedef enum SEM_TypeIsMutable{
	SEM_TYPE_CONST = 0
	SEM_TYPE_MUTABLE = 1
} SEM_TypeIsMutable;

typedef enum SEM_BasicTypeEnum{
	SEM_TYPE_BASIC_VOID = 0,
	SEM_TYPE_BASIC_INT,
	SEM_TYPE_BASIC_BOOL,
	SEM_TYPE_BASIC_FLOAT
} SEM_BasicTypeEnum;

typedef struct SEM_BasicType{
	SEM_BasicTypeEnum typeEnum;
} SEM_BasicType;

typedef struct SEM_ClassType{
	SEM_ClassDecl * classDecl;
} SEM_NamedType;

typedef struct SEM_PtrType{
	struct SEM_Type * ptrType;
} SEM_PtrType;

typedef struct SEM_Type{
	SEM_TypeEnum typeEnum;
	SEM_TypeIsMutable isMutable;
	
	union{
		SEM_BasicType basicType;
		SEM_ClassType classType;
		SEM_PtrType ptrType;
	};
} SEM_Type;

SEM_Type * SEM_MakeBasicType(SEM_TypeIsMutable isMutable, SEM_BasicTypeEnum typeEnum);

SEM_Type * SEM_MakeClassType(SEM_TypeIsMutable isMutable, SEM_ClassDecl * classDecl);

SEM_Type * SEM_MakePtrType(SEM_TypeIsMutable isMutable, SEM_Type * ptrType);

int SEM_CompareTypes(SEM_Type * firstType, SEM_Type * secondType);

int SEM_TypeIsNumeric(SEM_Type * type);

#endif
