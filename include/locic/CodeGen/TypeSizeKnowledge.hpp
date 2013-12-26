#ifndef LOCIC_CODEGEN_TYPESIZEKNOWLEDGE_HPP
#define LOCIC_CODEGEN_TYPESIZEKNOWLEDGE_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isTypeSizeKnownInThisModule(Module& module, SEM::Type* type);
		
		bool isTypeSizeAlwaysKnown(Module& module, SEM::Type* type);
		
	}
	
}

#endif
