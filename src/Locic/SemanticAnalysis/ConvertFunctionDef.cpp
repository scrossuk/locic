#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertScope.hpp>

namespace Locic{

namespace SemanticAnalysis{

SEM::FunctionDef* ConvertFunctionDef(ModuleContext& moduleContext, AST::FunctionDef* functionDef) {
	const std::string& functionName = functionDef->declaration->name;
	
	// Find the corresponding semantic function declaration.
	SEM::FunctionDecl* semFunctionDecl = moduleContext.getFunctionDecl(functionName);
	
	if(semFunctionDecl == NULL) {
		printf("Internal compiler error: semantic function declaration not found for definition '%s'.\n", functionName.c_str());
		return NULL;
	}
	
	LocalContext localContext(moduleContext, semFunctionDecl);
	
	// AST information gives parameter names; SEM information gives parameter variable information.
	const std::list<AST::TypeVar*>& astParameters = functionDef->declaration->parameters;
	const std::list<SEM::Var*>& semParameters = semFunctionDecl->parameters;
	
	assert(astParameters.size() == semParameters.size());
	
	std::list<AST::TypeVar*>::const_iterator astIterator;
	std::list<SEM::Var*>::const_iterator semIterator;
	
	for(astIterator = astParameters.begin(), semIterator = semParameters.begin();
	        astIterator != astParameters.end();
	        ++astIterator, ++semIterator) {
	        
		AST::TypeVar* typeVar = *astIterator;
		SEM::Var* paramVar = *semIterator;
		
		// Create a mapping from the parameter's name to its variable information.
		if(!localContext.defineFunctionParameter(typeVar->name, paramVar)) {
			printf("Semantic Analysis Error: cannot share names between function parameters.\n");
			return NULL;
		}
	}
	
	// Generate the outer function scope.
	// (which will then generate its contents etc.)
	SEM::Scope* scope = ConvertScope(localContext, functionDef->scope);
	
	if(scope == NULL) {
		return NULL;
	}
	
	SEM::Type* returnType = semFunctionDecl->type->functionType.returnType;
	
	if(!returnType->isVoid()) {
		// Functions with non-void return types must return a value.
		if(!WillScopeReturn(scope)) {
			printf("Semantic Analysis Error: Control reaches end of function with non-void return type (i.e. need to add a return statement).\n");
			return NULL;
		}
	} else {
		// Need to add a void return statement in case the program didn't.
		scope->statementList.push_back(SEM::Statement::ReturnVoid());
	}
	
	// Build and return the function definition.
	return new SEM::FunctionDef(NULL, semFunctionDecl, scope);
}

}

}

