#include <assert.h>

#include <map>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/Support/String.hpp>

#include <locic/AST/Scope.hpp>
#include <locic/SEM/SwitchCase.hpp>

namespace locic {

	namespace SEM {
	
		SwitchCase::SwitchCase()
		: var_(nullptr) { }
		
		SwitchCase::SwitchCase(AST::Var& pVar, AST::Node<AST::Scope> pScope)
		: var_(&pVar), scope_(std::move(pScope)) { }
		
		void SwitchCase::setVar(AST::Var& pVar) {
			assert(var_ == nullptr);
			var_ = &pVar;
		}
		
		void SwitchCase::setScope(AST::Node<AST::Scope> pScope) {
			assert(scope_.get() == nullptr);
			assert(pScope.get() != nullptr);
			scope_ = std::move(pScope);
		}
		
		AST::Var& SwitchCase::var() {
			return *var_;
		}
		
		const AST::Var& SwitchCase::var() const {
			return *var_;
		}
		
		FastMap<String, AST::Var*>& SwitchCase::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, AST::Var*>& SwitchCase::namedVariables() const {
			return namedVariables_;
		}
		
		AST::Scope& SwitchCase::scope() const {
			return *scope_;
		}
		
		std::string SwitchCase::toString() const {
			return makeString("SwitchCase(var: %s, scope: %s)",
				var().toString().c_str(),
				scope().toString().c_str());
		}
		
	}
	
}

