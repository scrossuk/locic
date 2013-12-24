#include <Locic/String.hpp>

#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/Scope.hpp>
#include <Locic/SEM/Statement.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/Value.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		std::string Statement::toString() const {
			switch(kind_) {
				case VALUE:
				{
					return makeString("ValueStatement(value: %s)",
						valueStmt_.value->toString().c_str());
				}
				case SCOPE:
				{
					return makeString("ScopeStatement(scope: %s)",
						scopeStmt_.scope->toString().c_str());
				}
				case INITIALISE:
				{
					return makeString("InitialiseStatement(var: %s, value: %s)",
						initialiseStmt_.var->toString().c_str(),
						initialiseStmt_.value->toString().c_str());
				}
				case IF:
				{
					return makeString("IfStatement(condition: %s, ifTrue: %s, ifFalse: %s)",
						ifStmt_.condition->toString().c_str(),
						ifStmt_.ifTrue->toString().c_str(),
						ifStmt_.ifFalse->toString().c_str());
				}
				case WHILE:
				{
					return makeString("WhileStatement(condition: %s, whileTrue: %s)",
						whileStmt_.condition->toString().c_str(),
						whileStmt_.whileTrue->toString().c_str());
				}
				case RETURN:
				{
					return makeString("ReturnStatement(value: %s)",
						returnStmt_.value == NULL ? "[VOID]" :
							returnStmt_.value->toString().c_str());
				}
				default:
					assert(false && "Unknown SEM::Statement kind.");
					return "Statement([INVALID])";
			}
		}
		
	}
	
}

