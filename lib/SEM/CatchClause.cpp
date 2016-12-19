#include <cassert>
#include <map>
#include <memory>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/Support/String.hpp>

#include <locic/SEM/CatchClause.hpp>
#include <locic/SEM/Scope.hpp>

namespace locic {

	namespace SEM {
	
		CatchClause::CatchClause() : var_(nullptr) { }
		
		void CatchClause::setVar(AST::Var& pVar) {
			assert(var_ == nullptr);
			var_ = &pVar;
		}
		
		void CatchClause::setScope(std::unique_ptr<Scope> pScope) {
			assert(scope_.get() == nullptr);
			assert(pScope.get() != nullptr);
			scope_ = std::move(pScope);
		}
		
		AST::Var& CatchClause::var() {
			return *var_;
		}
		
		const AST::Var& CatchClause::var() const {
			return *var_;
		}
		
		FastMap<String, AST::Var*>& CatchClause::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, AST::Var*>& CatchClause::namedVariables() const {
			return namedVariables_;
		}
		
		Scope& CatchClause::scope() const {
			return *scope_;
		}
		
		std::string CatchClause::toString() const {
			return makeString("CatchClause(var: %s, scope: %s)",
				var().toString().c_str(),
				scope().toString().c_str());
		}
		
	}
	
}

