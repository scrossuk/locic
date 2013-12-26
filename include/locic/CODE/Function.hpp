#ifndef LOCIC_CODE_FUNCTION_HPP
#define LOCIC_CODE_FUNCTION_HPP

#include <locic/Constant.hpp>
#include <locic/CODE/Type.hpp>

namespace locic {

	namespace CODE {
	
		struct Function {
			std::string name;
			Type * type;
			Var * argsVar;
			Var * returnVar;
			std::vector<Instruction *> instructions;
			
			inline Function()
				: type(NULL){ }
		};
	}
	
}

#endif
