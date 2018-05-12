#ifndef LOCIC_CODEGEN_LLVMINCLUDES_HPP
#define LOCIC_CODEGEN_LLVMINCLUDES_HPP

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/Triple.h>

#include <llvm/Analysis/Passes.h>

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>

#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DiagnosticInfo.h>
#include <llvm/IR/DiagnosticPrinter.h>
#include <llvm/IR/DIBuilder.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>

#include <llvm/IR/Attributes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include <llvm/IRReader/IRReader.h>

#include <llvm/Linker/Linker.h>

#include <llvm/MC/SubtargetFeature.h>

#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/Target/TargetMachine.h>

#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Scalar.h>

namespace locic {
	
	namespace CodeGen {
		
		using DICompileUnit = llvm::DICompileUnit*;
		using DIFile = llvm::DIFile*;
		using DILocalVariable = llvm::DILocalVariable*;
		using DINode = llvm::DINode;
		using DIScope = llvm::DIScope*;
		using DISubprogram = llvm::DISubprogram*;
		using DISubroutineType = llvm::DISubroutineType*;
		using DIType = llvm::DIType*;
		
		using LLVMMetadataValue = llvm::Metadata;
		
	}
	
}

#endif
