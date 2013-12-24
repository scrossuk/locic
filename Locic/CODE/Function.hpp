#ifndef LOCIC_CODE_FUNCTION_HPP
#define LOCIC_CODE_FUNCTION_HPP

#include <Locic/Constant.hpp>
#include <Locic/CODE/Type.hpp>

namespace Locic {

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
