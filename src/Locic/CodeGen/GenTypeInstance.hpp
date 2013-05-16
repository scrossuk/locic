#ifndef LOCIC_CODEGEN_GENTYPEINSTANCE_HPP
#define LOCIC_CODEGEN_GENTYPEINSTANCE_HPP

#include <vector>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
		
		llvm::StructType* genTypeInstance(Module& module, SEM::TypeInstance* typeInstance,
			const std::vector<SEM::Type*>& templateArguments);
		
	}
	
}

#endif
