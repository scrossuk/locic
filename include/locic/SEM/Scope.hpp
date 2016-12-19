#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <memory>
#include <string>

#include <locic/AST/ExitStates.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/FastMap.hpp>
#include <locic/Support/String.hpp>
#include <locic/SEM/Statement.hpp>

namespace locic {
	
	namespace AST {
		
		class Var;
		
	}
	
	namespace SEM {
		
		class Scope {
			public:
				static std::unique_ptr<Scope> Create();
				
				Scope();
				Scope(Scope&&) = default;
				Scope& operator=(Scope&&) = default;
				
				AST::ExitStates exitStates() const;
				
				Array<AST::Var*, 10>& variables();
				const Array<AST::Var*, 10>& variables() const;
				
				FastMap<String, AST::Var*>& namedVariables();
				const FastMap<String, AST::Var*>& namedVariables() const;
				
				Array<Statement, 10>& statements();
				const Array<Statement, 10>& statements() const;
				
				std::string toString() const;
				
			private:
				// Non-copyable.
				Scope(const Scope&);
				Scope& operator=(const Scope&);
				
				Array<AST::Var*, 10> variables_;
				FastMap<String, AST::Var*> namedVariables_;
				Array<Statement, 10> statementList_;
				
		};
		
	}
	
}

#endif
