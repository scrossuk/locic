#ifndef LOCIC_AST_TYPE_H
#define LOCIC_AST_TYPE_H

enum AST_TypeIsNamed{
	AST_TYPE_NAMED,
	AST_TYPE_PTR
};

enum AST_TypeIsMutable{
	AST_TYPE_MUTABLE,
	AST_TYPE_CONST
};

typedef struct AST_Type AST_Type;

typedef struct AST_NamedType{
	char * name;
} AST_NamedType;

typedef struct AST_PtrType{
	AST_Type * ptrType;
} AST_NamedType;

typedef struct AST_Type{
	AST_TypeIsNamed isNamed;
	AST_TypeIsMutable isMutable;
	
	union{
		AST_NamedType namedType;
		AST_PtrType ptrType;
	};
} AST_Type;

inline AST_Type * AST_AllocateType(AST_TypeIsNamed isNamed, AST_TypeIsMutable isMutable){
	AST_Type * type = malloc(sizeof(AST_Type));
	type->isNamed = isNamed;
	type->isMutable = isMutable;
	return type;
}

inline AST_Type * AST_MakeNamedType(AST_TypeIsMutable isMutable, char * name){
	AST_Type * type = AST_AllocateType(AST_TYPE_NAMED, isMutable);
	(type->namedType).name = name;
	return type;
}

inline AST_Type * AST_MakePtrType(AST_TypeIsMutable isMutable, AST_Type * ptrType){
	AST_Type * type = AST_AllocateType(AST_TYPE_PTR, isMutable);
	(type->ptrType).ptrype = ptrType;
	return type;
}

#endif
