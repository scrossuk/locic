#ifndef LOCIC_CODEGEN_DEFAULTMETHODS_HPP
#define LOCIC_CODEGEN_DEFAULTMETHODS_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		void genDefaultMethod(Function& functionGenerator, SEM::Type* parent, SEM::Function* function);
		
	}
	
}

#endif
