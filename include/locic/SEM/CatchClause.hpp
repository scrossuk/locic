#ifndef LOCIC_SEM_CATCHCLAUSE_HPP
#define LOCIC_SEM_CATCHCLAUSE_HPP

#include <memory>
#include <string>

#include <locic/AST/Node.hpp>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class Scope;
		class Var;
		
	}
	
	namespace SEM {
		
		class CatchClause {
			public:
				CatchClause();
				
				void setVar(AST::Var& var);
				
				void setScope(AST::Node<AST::Scope> scope);
				
				AST::Var& var();
				const AST::Var& var() const;
				
				FastMap<String, AST::Var*>& namedVariables();
				const FastMap<String, AST::Var*>& namedVariables() const;
				
				AST::Scope& scope() const;
				
				std::string toString() const;
				
			private:
				AST::Var* var_;
				FastMap<String, AST::Var*> namedVariables_;
				AST::Node<AST::Scope> scope_;
				
		};
		
	}
	
}

#endif
