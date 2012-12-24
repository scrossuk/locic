#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertScope.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		void ConvertFunctionDef(Context& context, AST::Function* astFunction, SEM::Function* semFunction) {
			if(semFunction->scope != NULL){
				throw MultipleFunctionDefinitionsException(semFunction->name);
			}
			
			LocalContext localContext(context, semFunction);
			
			// AST information gives parameter names; SEM information gives parameter variable information.
			const std::vector<AST::TypeVar*>& astParameters = astFunction->parameters;
			const std::vector<SEM::Var*>& semParameters = semFunction->parameters;
			
			assert(astParameters.size() == semParameters.size());
			
			for(std::size_t i = 0; i < astParameters.size(); i++){
				AST::TypeVar* typeVar = astParameters.at(i);
				SEM::Var* paramVar = semParameters.at(i);
				
				// Create a mapping from the parameter's name to its variable information.
				if(!localContext.defineFunctionParameter(typeVar->name, paramVar)) {
					throw ParamVariableClashException(semFunction->name, typeVar->name);
				}
			}
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			SEM::Scope* scope = ConvertScope(localContext, astFunction->scope);
			
			SEM::Type* returnType = semFunction->type->functionType.returnType;
			
			if(!returnType->isVoid()) {
				// Functions with non-void return types must return a value.
				if(!WillScopeReturn(scope)) {
					throw MissingReturnStatementException(semFunction->name);
				}
			} else {
				// Need to add a void return statement in case the program didn't.
				scope->statementList.push_back(SEM::Statement::ReturnVoid());
			}
			
			semFunction->scope = scope;
		}
		
	}
	
}

