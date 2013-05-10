#ifndef LOCIC_CODEGEN_GENTYPE_HPP
#define LOCIC_CODEGEN_GENTYPE_HPP

#include <string>
#include <vector>

#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::Type* genType(SEM::Type* type);
		
	}
	
}

#endif
