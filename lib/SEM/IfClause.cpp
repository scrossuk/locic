#include <assert.h>

#include <locic/AST/Value.hpp>

#include <locic/Support/String.hpp>

#include <locic/SEM/IfClause.hpp>
#include <locic/SEM/Scope.hpp>

namespace locic {

	namespace SEM {
	
		IfClause::IfClause(AST::Value pCondition, std::unique_ptr<Scope> pScope)
			: condition_(std::move(pCondition)), scope_(std::move(pScope)) { }
		
		const AST::Value& IfClause::condition() const {
			return condition_;
		}
		
		Scope& IfClause::scope() const {
			return *scope_;
		}
		
		std::string IfClause::toString() const {
			return makeString("IfClause(condition: %s, scope: %s)",
				condition().toString().c_str(),
				scope().toString().c_str());
		}
		
	}
	
}

