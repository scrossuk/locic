#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertType.h>

SEM_Type * Locic_SemanticAnalysis_ConvertType(Locic_SemanticContext * context, AST_Type * type, SEM_TypeIsLValue isLValue){
	switch(type->typeEnum){
		case AST_TYPE_BASIC:
		{
			return SEM_MakeBasicType(type->isMutable, isLValue, type->basicType.typeEnum);
		}
		case AST_TYPE_NAMED:
		{
			SEM_ClassDecl * classDecl = Locic_StringMap_Find(context->classDeclarations, type->namedType.name);
			if(classDecl == NULL){
				printf("Semantic Analysis Error: Unknown class type with name '%s'.\n", type->namedType.name);
				return NULL;
			}
			
			return SEM_MakeClassType(type->isMutable, isLValue, classDecl);
		}
		case AST_TYPE_PTR:
		{
			// Pointed-to types are always l-values (otherwise they couldn't have their address taken).
			SEM_Type * ptrType = Locic_SemanticAnalysis_ConvertType(context, type->ptrType.ptrType, SEM_TYPE_LVALUE);
			
			if(ptrType == NULL){
				return NULL;
			}
			
			return SEM_MakePtrType(type->isMutable, isLValue, ptrType);
		}
		case AST_TYPE_FUNC:
		{
			SEM_Type * returnType = Locic_SemanticAnalysis_ConvertType(context, type->funcType.returnType, SEM_TYPE_RVALUE);
			if(returnType == NULL){
				return NULL;
			}
			
			Locic_List * parameterTypes = Locic_List_Alloc();
			Locic_ListElement * it;
			for(it = Locic_List_Begin(type->funcType.parameterTypes); it != Locic_List_End(type->funcType.parameterTypes); it = it->next){
				SEM_Type * paramType = Locic_SemanticAnalysis_ConvertType(context, it->data, SEM_TYPE_LVALUE);
				if(paramType == NULL){
					return NULL;
				}
				Locic_List_Append(parameterTypes, paramType);
			}
			return SEM_MakeFuncType(type->isMutable, isLValue, returnType, parameterTypes);
		}
		default:
			printf("Internal Compiler Error: Unknown AST_Type type enum.\n");
			return NULL;
	}
}

