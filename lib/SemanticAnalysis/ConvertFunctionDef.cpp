#include <cassert>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void ConvertFunctionDef(Context& context) {
			Node& functionNode = context.node();
			
			// Look through all the AST functions corresponding to
			// this function to find the definition.
			const auto& astFunctionNode = functionNode.getASTFunction();
			
			if (astFunctionNode->typeEnum == AST::Function::DEFAULTDEFINITION) {
				// Has a default definition.
				return;
			}
			
			if (astFunctionNode->scope.get() == NULL) {
				// Only a declaration.
				return;
			}
			
			auto semFunction = functionNode.getSEMFunction();
			
			assert(astFunctionNode->scope.get() != NULL);
			
			// Function should currently be a declaration
			// (it is about to be made into a definition).
			assert(semFunction->isDeclaration());
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			auto semScope = ConvertScope(context, astFunctionNode->scope);
			
			auto returnType = semFunction->type()->getFunctionReturnType();
			
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

