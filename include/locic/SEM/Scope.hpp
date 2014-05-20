#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <map>
#include <string>
#include <vector>

namespace locic {

	namespace SEM {
		
		class Statement;
		class Var;
	
		class Scope {
			public:
				Scope();
				
				std::vector<Var*>& variables();
				const std::vector<Var*>& variables() const;
				
				std::map<std::string, Var*>& namedVariables();
				const std::map<std::string, Var*>& namedVariables() const;
				
				std::vector<Statement*>& statements();
				const std::vector<Statement*>& statements() const;
				
				std::string toString() const;
				
			private:
				std::vector<Var*> variables_;
				std::map<std::string, Var*> namedVariables_;
				std::vector<Statement*> statementList_;
				
		};
		
	}
	
}

#endif
