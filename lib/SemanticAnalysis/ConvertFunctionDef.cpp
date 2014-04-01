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
	
		void ConvertFunctionDef(Context& context) {
			const auto& functionNode = context.node();
			
			const auto& astFunctionNode = functionNode.getASTFunction();
			const auto semFunction = functionNode.getSEMFunction();
			
			// Function should currently be a declaration
			// (it is about to be made into a definition).
			assert(semFunction->isDeclaration());
			
			if (astFunctionNode.isNull() || astFunctionNode->isDefaultDefinition()) {
				// Has a default definition.
				CreateDefaultMethod(context, lookupParentType(context).getSEMTypeInstance(), semFunction, astFunctionNode.location());
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
			
			if (!returnType->isVoid()) {
				// Functions with non-void return types must return a value.
				if (!WillScopeReturn(*semScope)) {
					throw MissingReturnStatementException(context.name());
				}
			} else {
				// Need to add a void return statement in case the program didn't.
				semScope->statements().push_back(SEM::Statement::ReturnVoid());
			}
			
			semFunction->setScope(semScope);
		}
		
	}
	
}

