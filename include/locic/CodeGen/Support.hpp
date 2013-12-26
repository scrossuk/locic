#ifndef LOCIC_CODEGEN_SUPPORT_HPP
#define LOCIC_CODEGEN_SUPPORT_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <vector>

#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Type* voidType();
		
		llvm::Type* i8Type();
		
		llvm::Type* i32Type();
		
		llvm::Type* getSizeType(const TargetInfo& targetInfo);
		
		llvm::PointerType* i8PtrType();
		
		llvm::StructType* getVTableType(const TargetInfo& targetInfo);
		
	}
	
}

#endif
