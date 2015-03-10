#include <assert.h>

#include <map>

#include <locic/Support/String.hpp>

#include <locic/SEM/CatchClause.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		CatchClause::CatchClause()
			: var_(nullptr) { }
		
		void CatchClause::setVar(Var* pVar) {
			assert(pVar != nullptr);
			var_ = pVar;
		}
		
		void CatchClause::setScope(std::unique_ptr<Scope> pScope) {
			assert(scope_.get() == nullptr);
			assert(pScope.get() != nullptr);
			scope_ = std::move(pScope);
		}
		
		Var* CatchClause::var() const {
			return var_;
		}
		
		FastMap<String, Var*>& CatchClause::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, Var*>& CatchClause::namedVariables() const {
			return namedVariables_;
		}
		
		Scope& CatchClause::scope() const {
			return *scope_;
		}
		
		std::string CatchClause::toString() const {
			return makeString("CatchClause(var: %s, scope: %s)",
				var()->toString().c_str(),
				scope().toString().c_str());
		}
		
	}
	
}

