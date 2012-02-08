#ifndef LOCIC_AST_TYPE_H
#define LOCIC_AST_TYPE_H

#include <Locic/List.h>

typedef enum AST_TypeEnum{
	AST_TYPE_BASIC,
	AST_TYPE_NAMED,
	AST_TYPE_PTR,
	AST_TYPE_FUNC
} AST_TypeEnum;

typedef enum AST_TypeIsMutable{
	AST_TYPE_CONST = 0,
	AST_TYPE_MUTABLE = 1
} AST_TypeIsMutable;

typedef enum AST_BasicTypeEnum{
	AST_TYPE_BASIC_VOID = 0,
	AST_TYPE_BASIC_INT,
	AST_TYPE_BASIC_BOOL,
	AST_TYPE_BASIC_FLOAT
} AST_BasicTypeEnum;

typedef struct AST_BasicType{
	AST_BasicTypeEnum typeEnum;
} AST_BasicType;

typedef struct AST_NamedType{
	char * name;
} AST_NamedType;

typedef struct AST_PtrType{
	struct AST_Type * ptrType;
} AST_PtrType;

typedef struct AST_FuncType{
	struct AST_Type * returnType;
	Locic_List * parameterTypes;
} AST_FuncType;

typedef struct AST_Type{
	AST_TypeEnum typeEnum;
	AST_TypeIsMutable isMutable;
	
	union{
		AST_BasicType basicType;
		AST_NamedType namedType;
		AST_PtrType ptrType;
		AST_FuncType funcType;
	};
} AST_Type;

AST_Type * AST_MakeBasicType(AST_TypeIsMutable isMutable, AST_BasicTypeEnum typeEnum);

AST_Type * AST_MakeNamedType(AST_TypeIsMutable isMutable, char * name);

AST_Type * AST_MakePtrType(AST_Type * ptrType);

AST_Type * AST_MakeFuncType(AST_TypeIsMutable isMutable, AST_Type * returnType, Locic_List * parameterTypes);

AST_Type * AST_ApplyTransitiveConst(AST_Type * type);

#endif
