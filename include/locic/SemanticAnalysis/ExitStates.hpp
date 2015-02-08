#ifndef LOCIC_SEMANTICANALYSIS_EXITSTATES_HPP
#define LOCIC_SEMANTICANALYSIS_EXITSTATES_HPP

#include <bitset>

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		enum UnwindState {
			UnwindStateNormal = 0,
			UnwindStateReturn = 1,
			UnwindStateBreak = 2,
			UnwindStateContinue = 3,
			UnwindStateThrow = 4,
			UnwindStateRethrow = 5,
			UnwindState_MAX
		};
		
		std::bitset<UnwindState_MAX> GetScopeExitStates(const SEM::Scope& scope);
		
		std::bitset<UnwindState_MAX> GetStatementExitStates(SEM::Statement* statement);
		
		std::bitset<UnwindState_MAX> GetValueExitStates(const SEM::Value& value);
		
	}
	
}

#endif
