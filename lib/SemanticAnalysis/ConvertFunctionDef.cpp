#include <cassert>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void ConvertFunctionDef(Context& context, const AST::Node<AST::Function>& astFunctionNode) {
			const auto semFunction = context.scopeStack().back().function();
			
			// Function should currently be a declaration
			// (it is about to be made into a definition).
			assert(semFunction->isDeclaration());
			
			if (astFunctionNode->isDefaultDefinition()) {
				// Has a default definition.
				CreateDefaultMethod(context, lookupParentType(context.scopeStack()), semFunction, astFunctionNode.location());
				return;
			}
			
			if (astFunctionNode->isDeclaration()) {
				// Only a declaration.
				return;
			}
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			const auto semScope = ConvertScope(context, astFunctionNode->scope());
			
			const auto returnType = semFunction->type()->getFunctionReturnType();
			
			if (!WillScopeReturn(*semScope)) {
				if (!returnType->isBuiltInVoid()) {
					// Functions with non-void return types must return a value.
					throw MissingReturnStatementException(semFunction->name());
				} else {
					// Need to add a void return statement if the program didn't.
					semScope->statements().push_back(SEM::Statement::ReturnVoid());
				}
			}
			
			if (semFunction->type()->isFunctionNoExcept() && CanScopeThrow(*semScope)) {
				throw ErrorException(makeString("Function '%s' is declared as 'noexcept' but can throw.",
					semFunction->name().toString().c_str()));
			}
			
			semFunction->setScope(semScope);
		}
		
	}
	
}

