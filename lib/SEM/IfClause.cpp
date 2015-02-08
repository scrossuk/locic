#include <assert.h>

#include <locic/String.hpp>

#include <locic/SEM/IfClause.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Value.hpp>

namespace locic {

	namespace SEM {
	
		IfClause::IfClause(Value pCondition, std::unique_ptr<Scope> pScope)
			: condition_(std::move(pCondition)), scope_(std::move(pScope)) { }
		
		const Value& IfClause::condition() const {
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

