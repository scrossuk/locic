#ifndef LOCIC_SEM_TYPE_H
#define LOCIC_SEM_TYPE_H

#include <Locic/List.h>
#include <Locic/SEM/ClassDecl.h>

typedef enum SEM_TypeEnum{
	SEM_TYPE_BASIC,
	SEM_TYPE_CLASS,
	SEM_TYPE_PTR,
	SEM_TYPE_FUNC
} SEM_TypeEnum;

typedef enum SEM_TypeIsMutable{
	SEM_TYPE_CONST = 0,
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
} SEM_ClassType;

typedef struct SEM_PtrType{
	struct SEM_Type * ptrType;
} SEM_PtrType;

typedef struct SEM_FuncType{
	struct SEM_Type * returnType;
	Locic_List * parameterTypes;
} SEM_FuncType;

typedef enum SEM_TypeIsLValue{
	SEM_TYPE_LVALUE,
	SEM_TYPE_RVALUE
} SEM_TypeIsLValue;

typedef struct SEM_Type{
	SEM_TypeEnum typeEnum;
	SEM_TypeIsMutable isMutable;
	SEM_TypeIsLValue isLValue;
	
	union{
		SEM_BasicType basicType;
		SEM_ClassType classType;
		SEM_PtrType ptrType;
		SEM_FuncType funcType;
	};
} SEM_Type;

SEM_Type * SEM_MakeBasicType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_BasicTypeEnum typeEnum);

SEM_Type * SEM_MakeClassType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_ClassDecl * classDecl);

SEM_Type * SEM_MakePtrType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_Type * ptrType);

SEM_Type * SEM_MakeFuncType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_Type * returnType, Locic_List * parameterTypes);

SEM_Type * SEM_CopyType(SEM_Type * type);

int SEM_IsVoidType(SEM_Type * type);

#endif
