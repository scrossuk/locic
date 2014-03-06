#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertStatement.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Debug::StatementInfo makeStatementInfo(const AST::Node<AST::Statement>& astStatementNode) {
			Debug::StatementInfo statementInfo;
			statementInfo.location = astStatementNode.location();
			return statementInfo;
		}
		
		bool WillScopeReturn(const SEM::Scope& scope) {
			for(std::size_t i = 0; i < scope.statements().size(); i++) {
				if(WillStatementReturn(scope.statements().at(i))) {
					return true;
				}
			}
			
			return false;
		}
		
		SEM::Scope* ConvertScope(Context& context, AST::Node<AST::Scope> astScope) {
			assert(astScope.get() != nullptr);
			
			const auto semScope = new SEM::Scope();
			
			auto scopeNode = Node::Scope(astScope, semScope);
			NodeContext scopeContext(context, "##scope", scopeNode);
			
			// Go through each syntactic statement, and create a corresponding semantic statement.
			for (const auto& astStatementNode: *(astScope->statements)) {
				const auto statement = ConvertStatement(scopeContext, astStatementNode);
				assert(statement != nullptr);
				
				context.debugModule().statementMap.insert(std::make_pair(statement, makeStatementInfo(astStatementNode)));
				
				semScope->statements().push_back(statement);
			}
			
			return semScope;
		}
		
	}
	
}

