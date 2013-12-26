#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

	namespace CodeGen {
	
		void createPrimitiveSizeOf(Module& module, const std::string& name, const std::vector<SEM::Type*>& templateArguments, llvm::Function& llvmFunction);
		
		void createPrimitiveMethod(Module& module, SEM::Type* parent, SEM::Function* function, llvm::Function& llvmFunction);
		
		void createPrimitiveDestructor(Module& module, SEM::Type* parent, llvm::Function& llvmFunction);
								   
		llvm::Type* getPrimitiveType(const Module& module, const std::string& name, const std::vector<llvm::Type*>& templateArguments);
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, SEM::Type* type);
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, SEM::Type* type);
		
	}
	
}

#endif
