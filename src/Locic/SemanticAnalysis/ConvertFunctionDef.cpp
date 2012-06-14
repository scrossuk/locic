#include <cassert>
#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/StringMap.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertScope.h>

SEM::FunctionDef * Locic_SemanticAnalysis_ConvertFunctionDef(ModuleContext& moduleContext, AST::FunctionDef * functionDef){
	const std::string& funcName = functionDef->declaration->name;
	
	// Find the corresponding semantic function declaration.
	SEM::FunctionDecl * semFunctionDecl = moduleContext.getFunction(functionName);
	if(semFunctionDecl == NULL){
		printf("Internal compiler error: semantic function declaration not found for definition '%s'.\n", functionName.c_str());
		return NULL;
	}
	
	LocalContext localContext(moduleContext);
	
	// AST information gives parameter names; SEM information gives parameter variable information.
	const std::list<AST::TypeVar *>& synParameters = functionDef->declaration->parameters;
	const std::list<SEM::Var *>& semParameters = semFunctionDecl->parameterVars;
	
	assert(synParameters.size() == semParameters.size());
	
	std::list<AST::TypeVar *>::const_iterator synIterator;
	std::list<SEM::Var *>::const_iterator semIterator;
	
	for(synIterator = synParameters.begin(), semIterator = semParameters.begin();
		synIterator != synParameters.end();
		++synIterator, ++semIterator){
		
		AST::TypeVar * typeVar = *synIterator;
		SEM::Var * paramVar = *semIterator;
		
		// Create a mapping from the parameter's name to its variable information.
		if(!localContext.defineFunctionParameter(typeVar->name, paramVar)){
			printf("Semantic Analysis Error: cannot share names between function parameters.\n");
			return NULL;
		}
	}
	
	// Generate the outer function scope.
	// (which will then generate its contents etc.)
	SEM::Scope * scope = ConvertScope(localContext, functionDef->scope);
	
	if(scope == NULL){
		return NULL;
	}
	
	SEM::Type * returnType = semFunctionDecl->type->funcType.returnType;
	
	if(!returnType->isVoid()){
		// Functions with non-void return types must return a value.
		if(!WillScopeReturn(scope)){
			printf("Semantic Analysis Error: Control reaches end of function with non-void return type (i.e. need to add a return statement).\n");
			return NULL;
		}
	}else{
		// Need to add a void return statement in case the program didn't.
		scope->statementList.push_back(SEM::Statement::ReturnVoid());
	}
	
	// Build and return the function definition.
	return new SEM::FunctionDef(SEM::FunctionDef::NOPARENT, semFunctionDecl, scope);
}

