#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <memory>
#include <string>
#include <vector>

#include <locic/FastMap.hpp>
#include <locic/String.hpp>
#include <locic/SEM/ExitStates.hpp>

namespace locic {

	namespace SEM {
		
		class Statement;
		class Var;
	
		class Scope {
			public:
				static std::unique_ptr<Scope> Create();
				
				Scope();
				Scope(Scope&&) = default;
				Scope& operator=(Scope&&) = default;
				
				ExitStates exitStates() const;
				
				std::vector<Var*>& variables();
				const std::vector<Var*>& variables() const;
				
				FastMap<String, Var*>& namedVariables();
				const FastMap<String, Var*>& namedVariables() const;
				
				std::vector<Statement*>& statements();
				const std::vector<Statement*>& statements() const;
				
				std::string toString() const;
				
			private:
				// Non-copyable.
				Scope(const Scope&);
				Scope& operator=(const Scope&);
				
				std::vector<Var*> variables_;
				FastMap<String, Var*> namedVariables_;
				std::vector<Statement*> statementList_;
				
		};
		
	}
	
}

#endif
