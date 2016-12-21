#ifndef LOCIC_AST_SCOPE_HPP
#define LOCIC_AST_SCOPE_HPP

#include <locic/AST/ExitStates.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Statement.hpp>
#include <locic/AST/StatementDecl.hpp>

#include <locic/Support/Array.hpp>
#include <locic/Support/FastMap.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		class Scope {
		public:
			static Node<Scope>
			Create(const Debug::SourceLocation& location);
			
			Scope();
			Scope(Node<StatementDeclList> s);
			Scope(Scope&&) = default;
			Scope& operator=(Scope&&) = default;
			
			Node<StatementDeclList>& statementDecls();
			const Node<StatementDeclList>& statementDecls() const;
			
			ExitStates exitStates() const;
			
			Array<Var*, 10>& variables();
			const Array<Var*, 10>& variables() const;
			
			FastMap<String, Var*>& namedVariables();
			const FastMap<String, Var*>& namedVariables() const;
			
			Array<Statement, 10>& statements();
			const Array<Statement, 10>& statements() const;
			
			std::string toString() const;
			
		private:
			Node<StatementDeclList> statementDecls_;
			Array<Var*, 10> variables_;
			FastMap<String, Var*> namedVariables_;
			Array<Statement, 10> statements_;
			
		};
		
	}
	
}

#endif
