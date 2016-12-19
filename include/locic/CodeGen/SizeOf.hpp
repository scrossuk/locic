#ifndef LOCIC_CODEGEN_SIZEOF_HPP
#define LOCIC_CODEGEN_SIZEOF_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		ArgInfo alignMaskArgInfo(Module& module, const SEM::TypeInstance* typeInstance);
		
		ArgInfo sizeOfArgInfo(Module& module, const SEM::TypeInstance* typeInstance);
		
		ArgInfo memberOffsetArgInfo(Module& module, const SEM::TypeInstance* typeInstance);
		
		llvm::Function* genAlignMaskFunctionDecl(Module& module, const SEM::TypeInstance* typeInstance);
		
		llvm::Value* genAlignOf(Function& function, const AST::Type* type);
		
		llvm::Value* genAlignMask(Function& function, const AST::Type* type);
		
		llvm::Function* genSizeOfFunctionDecl(Module& module, const SEM::TypeInstance* typeInstance);
		
		llvm::Value* genSizeOf(Function& function, const AST::Type* type);
		
		llvm::Value* makeAligned(Function& function, llvm::Value* size, llvm::Value* alignMask);
		
		llvm::Value* genSuffixByteOffset(Function& function, const SEM::TypeInstance& typeInstance);
		
		llvm::Value* genMemberOffset(Function& function, const AST::Type* type, size_t memberIndex);
		
		llvm::Value* genMemberPtr(Function& function, llvm::Value* objectPointer, const AST::Type* objectType, size_t memberIndex);
		
		std::pair<llvm::Value*, llvm::Value*> getUnionDatatypePointers(Function& function, const AST::Type* type, llvm::Value* objectPointer);
		
	}
	
}

#endif
