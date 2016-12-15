#ifndef LOCIC_SEM_SWITCHCASE_HPP
#define LOCIC_SEM_SWITCHCASE_HPP

#include <memory>
#include <string>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class Var;
		
	}
	
	namespace SEM {
		
		class Scope;
		
		class SwitchCase {
			public:
				SwitchCase();
				SwitchCase(AST::Var& var, std::unique_ptr<Scope> scope);
				
				void setVar(AST::Var& var);
				
				void setScope(std::unique_ptr<Scope> scope);
				
				AST::Var& var();
				const AST::Var& var() const;
				
				FastMap<String, AST::Var*>& namedVariables();
				const FastMap<String, AST::Var*>& namedVariables() const;
				
				Scope& scope() const;
				
				std::string toString() const;
				
			private:
				AST::Var* var_;
				FastMap<String, AST::Var*> namedVariables_;
				std::unique_ptr<Scope> scope_;
				
		};
		
	}
	
}

#endif
