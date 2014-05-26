#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

	namespace CodeGen {
		
		class Function;
		
		bool isSignedIntegerType(const std::string& name);
		
		bool isUnsignedIntegerType(const std::string& name);
		
		bool isIntegerType(const std::string& name);
	
		void createPrimitiveSizeOf(Module& module, const std::string& name, const std::vector<SEM::Type*>& templateArguments, llvm::Function& llvmFunction);
		
		void createPrimitiveMethod(Module& module, SEM::Type* parent, SEM::Function* function, llvm::Function& llvmFunction);
		
		void createPrimitiveDestructor(Module& module, SEM::Type* parent, llvm::Function& llvmFunction);
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::Type* unresolvedType);
								   
		llvm::Type* getPrimitiveType(const Module& module, const std::string& name);
		
		bool primitiveTypeHasDestructor(Module& module, SEM::Type* type);
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, SEM::Type* type);
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, SEM::Type* type);
		
	}
	
}

#endif
