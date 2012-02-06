#include <stdio.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/CanCast.h>
#include <Locic/SemanticAnalysis/Context.h>

int Locic_SemanticAnalysis_CanDoImplicitCast(Locic_SemanticContext * context, SEM_Type * sourceType, SEM_Type * destType){
	if(sourceType->typeEnum != destType->typeEnum){
		return 0;
	}
	
	switch(sourceType->typeEnum){
		case SEM_TYPE_BASIC:
		{
			if(sourceType->basicType.typeEnum != destType->basicType.typeEnum){
				return 0;
			}
			return 1;
		}
		case SEM_TYPE_CLASS:
		{
			if(sourceType->classType.classDecl != destType->classType.classDecl){
				printf("Semantic Analysis Error: cannot convert between incompatible class types.\n");
				return 0;
			}
			
			// Check for const-correctness.
			if(sourceType->isMutable == SEM_TYPE_CONST && destType->isMutable == SEM_TYPE_MUTABLE){
				printf("Semantic Analysis Error: Const-correctness violation.\n");
				return 0;
			}
			
			if(sourceType->isLValue != destType->isLValue){
				printf("Semantic Analysis Error: cannot convert between l-values and r-values.\n");
				return 0;
			}
			
			return 1;
		}
		case SEM_TYPE_PTR:
		{
			// Check for const-correctness.
			if(sourceType->ptrType.ptrType->isMutable == SEM_TYPE_CONST && destType->ptrType.ptrType->isMutable == SEM_TYPE_MUTABLE){
				printf("Semantic Analysis Error: Const-correctness violation on pointer type.\n");
				return 0;
			}
			
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType->ptrType.ptrType, destType->ptrType.ptrType)){
				return 0;
			}
			return 1;
		}
		case SEM_TYPE_FUNC:
		{
			if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType->funcType.returnType, destType->funcType.returnType)){
				return 0;
			}
			
			Locic_List * sourceList = sourceType->funcType.parameterTypes;
			Locic_List * destList = destType->funcType.parameterTypes;
			
			if(Locic_List_Size(sourceList) != Locic_List_Size(destList)){
				return 0;
			}
			
			Locic_ListElement * sourceIt = Locic_List_Begin(sourceList);
			Locic_ListElement * destIt = Locic_List_Begin(destList);
			
			while(sourceIt != Locic_List_End(sourceList)){
				if(!Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceIt->data, destIt->data)){
					return 0;
				}
				sourceIt = sourceIt->next;
				destIt = destIt->next;
			}
			
			return 1;
		}
		default:
			return 0;
	}
}

int Locic_SemanticAnalysis_CanDoImplicitCopy(Locic_SemanticContext * context, SEM_Type * type){
	switch(type->typeEnum){
		case SEM_TYPE_BASIC:
		case SEM_TYPE_PTR:
		case SEM_TYPE_FUNC:
			// Basic, pointer and function types can be copied implicitly.
			return 1;
		default:
			return 0;
	}
}

int Locic_SemanticAnalysis_CanDoExplicitCast(Locic_SemanticContext * context, SEM_Type * sourceType, SEM_Type * destType){
	if(sourceType->typeEnum != destType->typeEnum){
		return 0;
	}
	
	switch(sourceType->typeEnum){
		case SEM_TYPE_BASIC:
		{
			if(sourceType->basicType.typeEnum == destType->basicType.typeEnum){
				return 1;
			}
			
			// Int -> Float.
			if(sourceType->basicType.typeEnum == SEM_TYPE_BASIC_INT && destType->basicType.typeEnum == SEM_TYPE_BASIC_FLOAT){
				return 1;
			}
			
			// Float -> Int.
			if(sourceType->basicType.typeEnum == SEM_TYPE_BASIC_FLOAT && destType->basicType.typeEnum == SEM_TYPE_BASIC_INT){
				return 1;
			}
			
			return 0;
		}
		case SEM_TYPE_CLASS:
		case SEM_TYPE_PTR:
		case SEM_TYPE_FUNC:
		{
			return Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType, destType);
		}
		default:
			return 0;
	}
}







