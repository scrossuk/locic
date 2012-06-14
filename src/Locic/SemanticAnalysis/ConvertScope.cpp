#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertStatement.hpp>

namespace Locic{

	namespace SemanticAnalysis{

bool CanScopeReturn(SEM_Scope * scope){
	std::list<SEM::Statement *>::const_iterator it;
	for(it = scope->statementList.begin(); it != scope->statementList.end(); ++it){
		if(CanStatementReturn(*it)) return true;
	}
	return false;
}

SEM_Scope * ConvertScope(LocalContext& context, AST::Scope * scope){
	SEM::Scope * semScope = new SEM::Scope();

	// Add this scope to the context, so that variables can be added to it.
	context.pushScope(semScope);
	
	// Go through each syntactic statement, and create a corresponding semantic statement.
	std::list<SEM::Statement *>::const_iterator it;
	for(it = scope->statementList.begin(); it != scope->statementList.end(); ++it){
		SEM::Statement * statement = ConvertStatement(context, *it);
		if(statement == NULL){
			return NULL;
		}
		
		semScope->statementList.push_back(statement);
	}
	
	// Remove this scope from the context.
	context.popScope();
	
	return semScope;
}

}

}

