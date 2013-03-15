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
	
		void ConvertFunctionDef(Context& context) {
			Node& functionNode = context.node();
			
			AST::Function * astFunction = functionNode.getASTFunction();
			SEM::Function * semFunction = functionNode.getSEMFunction();
			
			assert(astFunction->scope != NULL);
			
			if(semFunction->isDefinition()) {
				throw MultipleFunctionDefinitionsException(context.name());
			}
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			SEM::Scope* semScope = ConvertScope(context, astFunction->scope);
			
			SEM::Type* returnType = semFunction->type()->getFunctionReturnType();
			
			if(!returnType->isVoid()) {
				// Functions with non-void return types must return a value.
				if(!WillScopeReturn(*semScope)) {
					throw MissingReturnStatementException(context.name() + astFunction->name);
				}
			} else {
				// Need to add a void return statement in case the program didn't.
				semScope->statements().push_back(SEM::Statement::ReturnVoid());
			}
			
			semFunction->setScope(semScope);
		}
		
	}
	
}

