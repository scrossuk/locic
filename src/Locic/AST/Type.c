#include <stdlib.h>
#include <Locic/AST/Type.h>

inline AST_Type * AST_AllocateType(AST_TypeIsNamed isNamed, AST_TypeIsMutable isMutable){
	AST_Type * type = malloc(sizeof(AST_Type));
	type->isNamed = isNamed;
	type->isMutable = isMutable;
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

