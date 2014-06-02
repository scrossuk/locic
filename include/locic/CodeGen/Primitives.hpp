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
		
		void createPrimitiveAlignOf(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		
		void createPrimitiveSizeOf(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		
		void createPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function, llvm::Function& llvmFunction);
		
		void createPrimitiveDestructor(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::Type* varType);
		
		llvm::Type* getPrimitiveType(const Module& module, const std::string& name);
		
		bool primitiveTypeHasDestructor(Module& module, SEM::TypeInstance* typeInstance);
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, SEM::TypeInstance* typeInstance);
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, SEM::TypeInstance* typeInstance);
		
	}
	
}

#endif
