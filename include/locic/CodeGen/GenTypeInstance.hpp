#ifndef LOCIC_CODEGEN_GENTYPEINSTANCE_HPP
#define LOCIC_CODEGEN_GENTYPEINSTANCE_HPP

#include <vector>

#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::StructType* genTypeInstance(Module& module, SEM::TypeInstance* typeInstance,
			const std::vector<SEM::Type*>& templateArguments);
		
	}
	
}

#endif
