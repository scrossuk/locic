#ifndef LOCIC_CODEGEN_TYPESIZEKNOWLEDGE_HPP
#define LOCIC_CODEGEN_TYPESIZEKNOWLEDGE_HPP

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
		
		bool isTypeSizeKnownInThisModule(Module& module, SEM::Type* type);
		
		bool isTypeSizeAlwaysKnown(Module& module, SEM::Type* type);
		
	}
	
}

#endif
