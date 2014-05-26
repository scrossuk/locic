#ifndef LOCIC_CODEGEN_TEMPLATE_HPP
#define LOCIC_CODEGEN_TEMPLATE_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Function* genTemplateIntermediateFunction(Module& module, const std::vector<SEM::TemplateVar*>& templateVars, const std::vector<SEM::Type*>& templateUses);
		
		llvm::Function* genTemplateRootFunction(Module& module, const std::vector<SEM::Type*>& templateArguments);
		
		bool isRootTypeList(const std::vector<SEM::Type*>& templateArguments);
		
	}
	
}

#endif
