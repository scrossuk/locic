#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertStatement.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		std::unique_ptr<SEM::Scope> ConvertScope(Context& context, const AST::Node<AST::Scope>& astScope) {
			assert(astScope.get() != nullptr);
			
			auto semScope = SEM::Scope::Create();
			semScope->statements().reserve(astScope->statements->size());
			
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Scope(semScope.get()));
			
			// Go through each syntactic statement, and create a corresponding semantic statement.
			for (const auto& astStatementNode: *(astScope->statements)) {
				semScope->statements().push_back(ConvertStatement(context, astStatementNode));
			}
			
			// Check all variables are either used and not marked unused,
			// or are unused and marked as such.
			for (const auto& varPair: semScope->namedVariables()) {
				const auto& varName = varPair.first;
				const auto& var = varPair.second;
				if (var->isUsed() && var->isMarkedUnused()) {
					const auto& debugInfo = var->debugInfo();
					assert(debugInfo);
					const auto& location = debugInfo->declLocation;
					throw ErrorException(makeString("Local variable '%s' is marked as unused but is used in scope, at position %s.",
						varName.c_str(), location.toString().c_str()));
				} else if (!var->isUsed() && !var->isMarkedUnused()) {
					const auto& debugInfo = var->debugInfo();
					assert(debugInfo);
					const auto& location = debugInfo->declLocation;
					throw ErrorException(makeString("Local variable '%s' is unused, at position %s.",
						varName.c_str(), location.toString().c_str()));
				}
			}
			
			return semScope;
		}
		
	}
	
}

