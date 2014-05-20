#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertStatement.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		bool WillScopeReturn(const SEM::Scope& scope) {
			for (size_t i = 0; i < scope.statements().size(); i++) {
				if (WillStatementReturn(scope.statements().at(i))) {
					return true;
				}
			}
			
			return false;
		}
		
		bool CanScopeThrow(const SEM::Scope& scope) {
			for (size_t i = 0; i < scope.statements().size(); i++) {
				if (CanStatementThrow(scope.statements().at(i))) {
					return true;
				}
			}
			
			return false;
		}
		
		SEM::Scope* ConvertScope(Context& context, const AST::Node<AST::Scope>& astScope) {
			assert(astScope.get() != nullptr);
			
			const auto semScope = new SEM::Scope();
			
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Scope(semScope));
			
			// Go through each syntactic statement, and create a corresponding semantic statement.
			for (const auto& astStatementNode: *(astScope->statements)) {
				const auto statement = ConvertStatement(context, astStatementNode);
				assert(statement != nullptr);
				
				semScope->statements().push_back(statement);
			}
			
			return semScope;
		}
		
	}
	
}

