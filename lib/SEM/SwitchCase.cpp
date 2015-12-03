#include <assert.h>

#include <map>

#include <locic/Support/String.hpp>

#include <locic/SEM/Scope.hpp>
#include <locic/SEM/SwitchCase.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		SwitchCase::SwitchCase() { }
		
		SwitchCase::SwitchCase(std::unique_ptr<Var> pVar, std::unique_ptr<Scope> pScope)
		: var_(std::move(pVar)), scope_(std::move(pScope)) { }
		
		void SwitchCase::setVar(std::unique_ptr<Var> pVar) {
			assert(var_.get() == nullptr);
			assert(pVar != nullptr);
			var_ = std::move(pVar);
		}
		
		void SwitchCase::setScope(std::unique_ptr<Scope> pScope) {
			assert(scope_.get() == nullptr);
			assert(pScope.get() != nullptr);
			scope_ = std::move(pScope);
		}
		
		Var& SwitchCase::var() {
			return *var_;
		}
		
		const Var& SwitchCase::var() const {
			return *var_;
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
				var().toString().c_str(),
				scope().toString().c_str());
		}
		
	}
	
}

