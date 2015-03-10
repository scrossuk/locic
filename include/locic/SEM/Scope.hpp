#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <memory>
#include <string>

#include <locic/FastMap.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>
#include <locic/SEM/ExitStates.hpp>
#include <locic/SEM/Statement.hpp>

namespace locic {

	namespace SEM {
		
		class Var;
		
		class Scope {
			public:
				static std::unique_ptr<Scope> Create();
				
				Scope();
				Scope(Scope&&) = default;
				Scope& operator=(Scope&&) = default;
				
				ExitStates exitStates() const;
				
				Array<Var*, 10>& variables();
				const Array<Var*, 10>& variables() const;
				
				FastMap<String, Var*>& namedVariables();
				const FastMap<String, Var*>& namedVariables() const;
				
				Array<Statement, 10>& statements();
				const Array<Statement, 10>& statements() const;
				
				std::string toString() const;
				
			private:
				// Non-copyable.
				Scope(const Scope&);
				Scope& operator=(const Scope&);
				
				Array<Var*, 10> variables_;
				FastMap<String, Var*> namedVariables_;
				Array<Statement, 10> statementList_;
				
		};
		
	}
	
}

#endif
