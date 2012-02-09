#include <stdio.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/CanCast.h>
#include <Locic/SemanticAnalysis/Context.h>

SEM_Value * Locic_SemanticAnalysis_CastValueToType(Locic_SemanticContext * context, SEM_Value * value, SEM_Type * type){
	// Try a plain implicit cast.
	if(Locic_SemanticAnalysis_CanDoImplicitCast(context, value->type, type) == NULL){
		return value;
	}
			
	// Can't just cast from one type to the other =>
	// must attempt copying (to remove lvalue/const).
	if(Locic_SemanticAnalysis_CanDoImplicitCopy(context, value->type) == 1){
		// If possible, create a copy.
		SEM_Value * copiedValue = SEM_MakeCopyValue(value);
		const char * err = Locic_SemanticAnalysis_CanDoImplicitCast(context, copiedValue->type, type);
		if(err == NULL){
			// Copying worked.
			return copiedValue;
		}
		printf("%s", err);
	}
	
	// Copying also failed.
	return NULL;
}

const char * Locic_SemanticAnalysis_CanDoImplicitCast(Locic_SemanticContext * context, SEM_Type * sourceType, SEM_Type * destType){
	if(sourceType->typeEnum != destType->typeEnum){
		return "Semantic Analysis Error: Types don't match.\n";
	}
	
	// Check for const-correctness.
	if(sourceType->isMutable == SEM_TYPE_CONST && destType->isMutable == SEM_TYPE_MUTABLE){
		return "Semantic Analysis Error: Const-correctness violation.\n";
	}
	
	if(sourceType->isLValue == SEM_TYPE_LVALUE && destType->isLValue == SEM_TYPE_RVALUE){
		return "Semantic Analysis Error: Cannot cast from lvalue to rvalue.\n";
	}
	
	switch(sourceType->typeEnum){
		case SEM_TYPE_BASIC:
		{
			if(sourceType->basicType.typeEnum != destType->basicType.typeEnum){
				return "Semantic Analysis Error: Cannot implicitly convert between different basic types.\n";
			}
			return NULL;
		}
		case SEM_TYPE_NAMED:
		{
			if(sourceType->namedType.typeInstance != destType->namedType.typeInstance){
				return "Semantic Analysis Error: Cannot convert between incompatible named types.\n";
			}
			return NULL;
		}
		case SEM_TYPE_PTR:
		{
			// Check for const-correctness inside pointers (e.g. to prevent T** being cast to const T**).
			if(sourceType->ptrType.ptrType->typeEnum == SEM_TYPE_PTR && destType->ptrType.ptrType->typeEnum == SEM_TYPE_PTR){
				SEM_Type * sourcePtrType = sourceType->ptrType.ptrType->ptrType.ptrType;
				SEM_Type * destPtrType = destType->ptrType.ptrType->ptrType.ptrType;
				if(sourcePtrType->isMutable == SEM_TYPE_MUTABLE && destPtrType->isMutable == SEM_TYPE_CONST){
					if(sourceType->ptrType.ptrType->isMutable == SEM_TYPE_MUTABLE && destType->ptrType.ptrType->isMutable == SEM_TYPE_MUTABLE){
						return "Semantic Analysis Error: Const-correctness violation on pointer type.\n";
					}
				}
			}
			
			return Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType->ptrType.ptrType, destType->ptrType.ptrType);
		}
		case SEM_TYPE_FUNC:
		{
			const char * err = Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType->funcType.returnType, destType->funcType.returnType);
			if(err != NULL) return err;
			
			Locic_List * sourceList = sourceType->funcType.parameterTypes;
			Locic_List * destList = destType->funcType.parameterTypes;
			
			if(Locic_List_Size(sourceList) != Locic_List_Size(destList)){
				return "Semantic Analysis Error: Number of parameters doesn't match in function type.\n";
			}
			
			Locic_ListElement * sourceIt = Locic_List_Begin(sourceList);
			Locic_ListElement * destIt = Locic_List_Begin(destList);
			
			while(sourceIt != Locic_List_End(sourceList)){
				if(Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceIt->data, destIt->data) != NULL){
					return "Semantic Analysis Error: Cannot cast parameter type in function type.\n";
				}
				sourceIt = sourceIt->next;
				destIt = destIt->next;
			}
			
			return NULL;
		}
		default:
			return "Internal Compiler Error: Unknown type enum value.";
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
		case SEM_TYPE_NAMED:
		case SEM_TYPE_PTR:
		case SEM_TYPE_FUNC:
		{
			const char * err = Locic_SemanticAnalysis_CanDoImplicitCast(context, sourceType, destType);
			if(err == NULL){
				return 1;
			}else{
				printf("%s", err);
				return 0;
			}
		}
		default:
			return 0;
	}
}







