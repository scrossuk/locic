#include <assert.h>

#include <map>

#include <locic/Support/String.hpp>

#include <locic/SEM/Scope.hpp>
#include <locic/SEM/SwitchCase.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		SwitchCase::SwitchCase()
			: var_(nullptr), scope_(nullptr) { }
		
		SwitchCase::SwitchCase(Var* pVar, std::unique_ptr<Scope> pScope)
			: var_(pVar), scope_(std::move(pScope)) { }
		
		void SwitchCase::setVar(Var* pVar) {
			assert(pVar != nullptr);
			var_ = pVar;
		}
		
		void SwitchCase::setScope(std::unique_ptr<Scope> pScope) {
			assert(scope_.get() == nullptr);
			assert(pScope.get() != nullptr);
			scope_ = std::move(pScope);
		}
		
		Var* SwitchCase::var() const {
			return var_;
		}
		
		FastMap<String, Var*>& SwitchCase::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, Var*>& SwitchCase::namedVariables() const {
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

