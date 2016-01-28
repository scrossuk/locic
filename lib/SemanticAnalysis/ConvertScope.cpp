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
		
		class UnusedLocalVarDiag: public Warning {
		public:
			UnusedLocalVarDiag(String varName)
			: varName_(varName) { }
			
			std::string toString() const {
				return makeString("unused variable '%s' (which is not marked as 'unused')",
				                  varName_.c_str());
			}
			
		private:
			String varName_;
			
		};
		
		class UsedLocalVarMarkedUnusedDiag: public Warning {
		public:
			UsedLocalVarMarkedUnusedDiag(String varName)
			: varName_(varName) { }
			
			std::string toString() const {
				return makeString("variable '%s' is marked 'unused' but is used",
				                  varName_.c_str());
			}
			
		private:
			String varName_;
			
		};
		
		std::unique_ptr<SEM::Scope> ConvertScope(Context& context, const AST::Node<AST::Scope>& astScope) {
			assert(astScope.get() != nullptr);
			
			auto semScope = SEM::Scope::Create();
			semScope->statements().reserve(astScope->statements->size());
			
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Scope(*semScope));
			
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
					context.issueDiag(UsedLocalVarMarkedUnusedDiag(varName),
					                  location);
				} else if (!var->isUsed() && !var->isMarkedUnused()) {
					const auto& debugInfo = var->debugInfo();
					assert(debugInfo);
					const auto& location = debugInfo->declLocation;
					context.issueDiag(UnusedLocalVarDiag(varName),
					                  location);
				}
			}
			
			return semScope;
		}
		
	}
	
}

