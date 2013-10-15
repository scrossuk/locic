#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertScope.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

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
					throw MissingReturnStatementException(context.name() + astFunctionNode->name);
				}
			} else {
				// Need to add a void return statement in case the program didn't.
				semScope->statements().push_back(SEM::Statement::ReturnVoid());
			}
			
			semFunction->setScope(semScope);
		}
		
	}
	
}

