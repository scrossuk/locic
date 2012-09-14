#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertScope.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Function * ConvertFunctionDef(Context& context, AST::Function* function, bool isMethod) {
			const std::string functionName = function->getFullName();
			
			// Find the corresponding semantic function
			// (which MUST have been created previously).
			SEM::Function* semFunction = context.getFunction(function->name);
			
			assert(semFunction != NULL);
			
			if(semFunction->scope != NULL){
				printf("Semantic Analysis Error: function '%s' was defined more than once.\n", functionName.c_str());
				return NULL;
			}
			
			LocalContext localContext(context, semFunction);
			
			// AST information gives parameter names; SEM information gives parameter variable information.
			const std::vector<AST::TypeVar*>& astParameters = function->parameters;
			const std::vector<SEM::Var*>& semParameters = semFunction->parameters;
			
			const std::size_t paramOffset = isMethod ? 1 : 0;
			
			assert(astParameters.size() + paramOffset == semParameters.size());
			
			for(std::size_t i = 0; i < astParameters.size(); i++){
				AST::TypeVar* typeVar = astParameters.at(i);
				SEM::Var* paramVar = semParameters.at(paramOffset + i);
				
				// Create a mapping from the parameter's name to its variable information.
				if(!localContext.defineFunctionParameter(typeVar->name, paramVar)) {
					printf("Semantic Analysis Error: cannot share names between function parameters.\n");
					return NULL;
				}
			}
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			SEM::Scope* scope = ConvertScope(localContext, function->scope);
			
			if(scope == NULL) {
				return NULL;
			}
			
			SEM::Type* returnType = semFunction->type->functionType.returnType;
			
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
			
			semFunction->scope = scope;
			return semFunction;
		}
		
	}
	
}

