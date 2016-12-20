#include <map>

#include <locic/AST/Scope.hpp>
#include <locic/AST/SwitchCase.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		SwitchCase::SwitchCase(Node<Var> pVar, Node<Scope> pScope)
		: var_(std::move(pVar)), scope_(std::move(pScope)) { }
		
		Node<Var>& SwitchCase::var() {
			return var_;
		}
		
		const Node<Var>& SwitchCase::var() const {
			return var_;
		}
		
		Node<Scope>& SwitchCase::scope() {
			return scope_;
		}
		
		const Node<Scope>& SwitchCase::scope() const {
			return scope_;
		}
		
		FastMap<String, Var*>& SwitchCase::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, Var*>& SwitchCase::namedVariables() const {
			return namedVariables_;
		}
		
		std::string SwitchCase::toString() const {
			return makeString("SwitchCase(var: %s, scope: %s)",
				var()->toString().c_str(),
				scope()->toString().c_str());
		}
		
		DefaultCase* DefaultCase::Empty() {
			return new DefaultCase(Node<Scope>());
		}
		
		DefaultCase* DefaultCase::ScopeCase(Node<Scope> scope) {
			return new DefaultCase(std::move(scope));
		}
		
		bool DefaultCase::hasScope() const {
			return scope_.get() != nullptr;
		}
		
		Node<Scope>& DefaultCase::scope() {
			assert(hasScope());
			return scope_;
		}
		
		const Node<Scope>& DefaultCase::scope() const {
			assert(hasScope());
			return scope_;
		}
		
		DefaultCase::DefaultCase(Node<Scope> pScope)
		: scope_(std::move(pScope)) { }
		
	}
	
}

