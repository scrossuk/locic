#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertType.h>

SEM_FunctionDecl * Locic_SemanticAnalysis_ConvertFunctionDecl(Locic_SemanticContext * context, AST_FunctionDecl * functionDecl){
	AST_Type * returnType = functionDecl->returnType;
	
	// Return types are always rvalues.
	SEM_Type * semReturnType = Locic_SemanticAnalysis_ConvertType(context, returnType, SEM_TYPE_RVALUE);
	
	if(semReturnType == NULL){
		return NULL;
	}
	
	size_t id = 0;
	Locic_List * parameterVars = Locic_List_Alloc();
	Locic_List * parameterTypes = Locic_List_Alloc();
	
	Locic_ListElement * it;
	for(it = Locic_List_Begin(functionDecl->parameters); it != Locic_List_End(functionDecl->parameters); it = it->next, id++){
		AST_TypeVar * typeVar = it->data;
		AST_Type * paramType = typeVar->type;
		
		// Parameter types are always lvalues.
		SEM_Type * semParamType = Locic_SemanticAnalysis_ConvertType(context, paramType, SEM_TYPE_LVALUE);
		
		if(semParamType == NULL){
			return NULL;
		}
		
		SEM_Var * semParamVar = SEM_MakeVar(SEM_VAR_PARAM, id, semParamType);
		
		Locic_List_Append(parameterTypes, semParamType);
		Locic_List_Append(parameterVars, semParamVar);
	}
	
	SEM_Type * functionType = SEM_MakeFuncType(SEM_TYPE_MUTABLE, SEM_TYPE_RVALUE, semReturnType, parameterTypes);
	
	return SEM_MakeFunctionDecl(NULL, functionType, functionDecl->name, parameterVars);
}


