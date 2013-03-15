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
		
		SEM::Scope* ConvertScope(Context& context, AST::Scope* astScope) {
			assert(astScope != NULL);
			
			SEM::Scope* semScope = new SEM::Scope();
			
			Node scopeNode = Node::Scope(astScope, semScope);
			Context scopeContext(context, "##scope", scopeNode);
			
			// Go through each syntactic statement, and create a corresponding semantic statement.
			for(std::size_t i = 0; i < astScope->statements.size(); i++) {
				SEM::Statement* statement = ConvertStatement(scopeContext, astScope->statements.at(i));
				assert(statement != NULL);
				
				semScope->statements().push_back(statement);
			}
			
			return semScope;
		}
		
	}
	
}

