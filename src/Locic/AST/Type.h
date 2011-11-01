#ifndef LOCIC_AST_TYPE_H
#define LOCIC_AST_TYPE_H

typedef enum AST_TypeIsNamed{
	AST_TYPE_NAMED,
	AST_TYPE_PTR
} AST_TypeIsNamed;

typedef enum AST_TypeIsMutable{
	AST_TYPE_MUTABLE,
	AST_TYPE_CONST
} AST_TypeIsMutable;

typedef struct AST_NamedType{
	char * name;
} AST_NamedType;

typedef struct AST_PtrType{
	struct AST_Type * ptrType;
} AST_PtrType;

typedef struct AST_Type{
	AST_TypeIsNamed isNamed;
	AST_TypeIsMutable isMutable;
	
	union{
		AST_NamedType namedType;
		AST_PtrType ptrType;
	};
} AST_Type;

AST_Type * AST_MakeNamedType(AST_TypeIsMutable isMutable, char * name);

AST_Type * AST_MakePtrType(AST_TypeIsMutable isMutable, AST_Type * ptrType);

#endif
