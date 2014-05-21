#include <assert.h>

#include <map>

#include <locic/String.hpp>

#include <locic/SEM/CatchClause.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		CatchClause::CatchClause()
			: var_(nullptr), scope_(nullptr) { }
		
		void CatchClause::setVar(Var* pVar) {
			assert(pVar != nullptr);
			var_ = pVar;
		}
		
		void CatchClause::setScope(Scope* pScope) {
			assert(pScope != nullptr);
			scope_ = pScope;
		}
		
		Var* CatchClause::var() const {
			return var_;
		}
		
		std::map<std::string, Var*>& CatchClause::namedVariables() {
			return namedVariables_;
		}
		
		const std::map<std::string, Var*>& CatchClause::namedVariables() const {
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

