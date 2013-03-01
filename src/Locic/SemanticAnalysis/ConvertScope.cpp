#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertStatement.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool WillScopeReturn(const SEM::Scope& scope) {
			for(std::size_t i = 0; i < scope.statements().size(); i++) {
				if(WillStatementReturn(scope.statements().at(i))) {
					return true;
				}
			}
			
			return false;
		}
		
		struct PushScope {
			LocalContext& context;
			
			inline PushScope(LocalContext& c, SEM::Scope* scope)
				: context(c) {
				context.pushScope(scope);
			}
			
			inline ~PushScope() {
				context.popScope();
			}
		};
		
		SEM::Scope* ConvertScope(LocalContext& context, AST::Scope* scope) {
			SEM::Scope* semScope = new SEM::Scope();
			
			// Add this scope to the context, so that variables can be added to it.
			PushScope pushScope(context, semScope);
			
			// Go through each syntactic statement, and create a corresponding semantic statement.
			for(std::size_t i = 0; i < scope->statements.size(); i++) {
				SEM::Statement* statement = ConvertStatement(context, scope->statements.at(i));
				assert(statement != NULL);
				
				semScope->statements().push_back(statement);
			}
			
			return semScope;
		}
		
	}
	
}

