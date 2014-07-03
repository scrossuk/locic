#ifndef LOCIC_CODEGEN_UNWINDSTATE_HPP
#define LOCIC_CODEGEN_UNWINDSTATE_HPP

namespace locic {

	namespace CodeGen {
	
		enum UnwindState {
			UnwindStateNormal = 0,
			UnwindStateReturn = 1,
			UnwindStateBreak = 2,
			UnwindStateContinue = 3,
			UnwindStateThrow = 4,
			UnwindStateRethrow = 5,
			UnwindState_MAX
		};
		
	}
	
}

#endif
