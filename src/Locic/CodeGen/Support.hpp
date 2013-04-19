#ifndef LOCIC_CODEGEN_SUPPORT_HPP
#define LOCIC_CODEGEN_SUPPORT_HPP

#include <llvm/Type.h>

#include <vector>

#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

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
