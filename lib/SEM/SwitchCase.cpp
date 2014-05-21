#include <assert.h>

#include <map>

#include <locic/String.hpp>

#include <locic/SEM/Scope.hpp>
#include <locic/SEM/SwitchCase.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		SwitchCase::SwitchCase()
			: var_(nullptr), scope_(nullptr) { }
		
		SwitchCase::SwitchCase(Var* pVar, Scope* pScope)
			: var_(pVar), scope_(pScope) { }
		
		void SwitchCase::setVar(Var* pVar) {
			assert(pVar != nullptr);
			var_ = pVar;
		}
		
		void SwitchCase::setScope(Scope* pScope) {
			assert(pScope != nullptr);
			scope_ = pScope;
		}
		
		Var* SwitchCase::var() const {
			return var_;
		}
		
		std::map<std::string, Var*>& SwitchCase::namedVariables() {
			return namedVariables_;
		}
		
		const std::map<std::string, Var*>& SwitchCase::namedVariables() const {
			return namedVariables_;
		}
		
		Scope& SwitchCase::scope() const {
			return *scope_;
		}
		
		std::string SwitchCase::toString() const {
			return makeString("SwitchCase(var: %s, scope: %s)",
				var()->toString().c_str(),
				scope().toString().c_str());
		}
		
	}
	
}

