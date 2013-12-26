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
			AST::FunctionList astFunctionDefNodeList;
			for (const auto& astFunctionNode: functionNode.getASTFunctionList()) {
				if (astFunctionNode->scope.get() != NULL) {
					astFunctionDefNodeList.push_back(astFunctionNode);
				}
			}
			
			if (astFunctionDefNodeList.empty()) {
				// Only a declaration.
				return;
			}
			
			if (astFunctionDefNodeList.size() > 1) {
				// More than one definition!
				throw MultipleFunctionDefinitionsException(context.name());
			}
			
			const AST::Node<AST::Function> astFunctionNode = astFunctionDefNodeList.front();
			SEM::Function * semFunction = functionNode.getSEMFunction();
			
			assert(astFunctionNode->scope.get() != NULL);
			assert(!semFunction->isDefinition());
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			SEM::Scope* semScope = ConvertScope(context, astFunctionNode->scope);
			
			SEM::Type* returnType = semFunction->type()->getFunctionReturnType();
			
			if (!returnType->isVoid()) {
				// Functions with non-void return types must return a value.
				if(!WillScopeReturn(*semScope)) {
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

