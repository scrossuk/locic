#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>

SEM::FunctionDecl * ConvertFunctionDecl(GlobalContext& context, AST::FunctionDecl * functionDecl){
	AST::Type * returnType = functionDecl->returnType;
	
	// Return types are always rvalues.
	SEM::Type * semReturnType = ConvertType(context, returnType, SEM::Type::RVALUE);
	
	if(semReturnType == NULL){
		return NULL;
	}
	
	size_t id = 0;
	std::list<SEM::Var *> parameterVars;
	std::list<SEM::Type *> parameterTypes;
	
	std::list<AST::TypeVar *>::const_iterator it;
	for(it = functionDecl->parameters.begin(); it != functionDecl->parameters.end(); ++it, id++){
		AST::TypeVar * typeVar = *it;
		AST::Type * paramType = typeVar->type;
		
		// Parameter types are always lvalues.
		SEM::Type * semParamType = ConvertType(context, paramType, SEM::Type::LVALUE);
		
		if(semParamType == NULL){
			return NULL;
		}
		
		if(semParamType->typeEnum == SEM::Type::VOID){
			printf("Semantic Analysis Error: Parameter variable cannot have void type.\n");
			return NULL;
		}
		
		SEM::Var * semParamVar = new SEM::Var(SEM::Var::PARAM, id, semParamType);
		
		parameterTypes.push_back(semParamType);
		parameterVars.push_back(semParamVar);
	}
	
	SEM::Type * functionType = SEM::Type::Function(SEM::Type::MUTABLE, SEM::Type::RVALUE, semReturnType, parameterTypes);
	
	return new SEM::FunctionDecl(SEM::FunctionDecl::NOPARENT, functionType, functionDecl->name, parameterVars);
}


