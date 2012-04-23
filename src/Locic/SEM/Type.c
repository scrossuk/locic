#include <assert.h>
#include <stdio.h>
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

SEM_Type * SEM_MakeVoidType(SEM_TypeIsMutable isMutable){
	return SEM_AllocateType(SEM_TYPE_VOID, isMutable, SEM_TYPE_RVALUE);
}

SEM_Type * SEM_MakeNullType(SEM_TypeIsMutable isMutable){
	return SEM_AllocateType(SEM_TYPE_NULL, isMutable, SEM_TYPE_RVALUE);
}

SEM_Type * SEM_MakeBasicType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_BasicTypeEnum typeEnum){
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_BASIC, isMutable, isLValue);
	(type->basicType).typeEnum = typeEnum;
	return type;
}

SEM_Type * SEM_MakeNamedType(SEM_TypeIsMutable isMutable, SEM_TypeIsLValue isLValue, SEM_TypeInstance * typeInstance){
	assert(typeInstance != NULL);
	SEM_Type * type = SEM_AllocateType(SEM_TYPE_NAMED, isMutable, isLValue);
	(type->namedType).typeInstance = typeInstance;
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
		case SEM_TYPE_NAMED:
			newType->namedType = type->namedType;
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
	if(type->typeEnum != SEM_TYPE_VOID) return 0;
	return 1;
}

void SEM_PrintType(SEM_Type * type){
	int bracket = 0;
	if(type->isMutable == SEM_TYPE_CONST){
		printf("const (");
		bracket = 1;
	}
	
	if(type->isLValue == SEM_TYPE_LVALUE){
		if(!bracket) printf("(");
		bracket = 1;
		printf("lvalue ");
	}
	
	switch(type->typeEnum){
		case SEM_TYPE_VOID:
		{
			printf("void");
			break;
		}
		case SEM_TYPE_NULL:
		{
			printf("null");
			break;
		}
		case SEM_TYPE_BASIC:
		{
			switch(type->basicType.typeEnum){
				case SEM_TYPE_BASIC_BOOL:
					printf("bool");
					break;
				case SEM_TYPE_BASIC_INT:
					printf("int");
					break;
				case SEM_TYPE_BASIC_FLOAT:
					printf("float");
					break;
				default:
					printf("[unknown basic]");
					break;
			}
			break;
		}
		case SEM_TYPE_NAMED:
			printf("[named type]");
			break;
		case SEM_TYPE_PTR:
			SEM_PrintType(type->ptrType.ptrType);
			printf(" *");
			break;
		case SEM_TYPE_FUNC:
		{
			printf("(");
			SEM_PrintType(type->funcType.returnType);
			printf(")(");
			
			Locic_ListElement * it;
			for(it = Locic_List_Begin(type->funcType.parameterTypes); it != Locic_List_End(type->funcType.parameterTypes); it = it->next){
				if(it != Locic_List_Begin(type->funcType.parameterTypes)){
					printf(", ");
				}
				SEM_PrintType(it->data);
			}
			
			printf(")");
			break;
		}
		default:
			break;
	}
	
	if(bracket){
		printf(")");
	}
}

