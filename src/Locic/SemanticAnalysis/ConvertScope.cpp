#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertStatement.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool WillScopeReturn(SEM::Scope* scope) {
			for(std::size_t i = 0; i < scope->statementList.size(); i++){
				if(WillStatementReturn(scope->statementList.at(i))) {
					return true;
				}
			}
			
			return false;
		}
		
		SEM::Scope* ConvertScope(LocalContext& context, AST::Scope* scope) {
			SEM::Scope* semScope = new SEM::Scope();
			
			// Add this scope to the context, so that variables can be added to it.
			context.pushScope(semScope);
			
			// Go through each syntactic statement, and create a corresponding semantic statement.
			for(std::size_t i = 0; i < scope->statements.size(); i++){
				SEM::Statement* statement = ConvertStatement(context, scope->statements.at(i));
				
				if(statement == NULL) {
					context.popScope();
					return NULL;
				}
				
				semScope->statementList.push_back(statement);
			}
			
			// Remove this scope from the context.
			context.popScope();
			
			return semScope;
		}
		
	}
	
}

