#include <cassert>
#include <map>
#include <memory>

#include <locic/AST/CatchClause.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		CatchClause::CatchClause(Node<Var> pVar, Node<Scope> pScope)
		: var_(std::move(pVar)), scope_(std::move(pScope)) { }
		
		Node<Var>& CatchClause::var() {
			return var_;
		}
		
		const Node<Var>& CatchClause::var() const {
			return var_;
		}
		
		Node<Scope>& CatchClause::scope() {
			return scope_;
		}
		
		const Node<Scope>& CatchClause::scope() const {
			return scope_;
		}
		
		FastMap<String, Var*>& CatchClause::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, Var*>& CatchClause::namedVariables() const {
			return namedVariables_;
		}
		
		std::string CatchClause::toString() const {
			return makeString("CatchClause(var: %s, scope: %s)",
				var()->toString().c_str(),
				scope()->toString().c_str());
		}
		
	}
	
}

