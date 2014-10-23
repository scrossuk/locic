#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertStatement.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		std::unique_ptr<SEM::Scope> ConvertScope(Context& context, const AST::Node<AST::Scope>& astScope) {
			assert(astScope.get() != nullptr);
			
			auto semScope = SEM::Scope::Create();
			semScope->statements().reserve(astScope->statements->size());
			
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Scope(semScope.get()));
			
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

