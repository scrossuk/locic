#ifndef LOCIC_CODEGEN_LLVMINCLUDES_HPP
#define LOCIC_CODEGEN_LLVMINCLUDES_HPP

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Analysis/Passes.h>

#if LOCIC_LLVM_VERSION >= 305
#include <llvm/IR/Verifier.h>
#else
#include <llvm/Analysis/Verifier.h>
#endif

#include <llvm/Bitcode/ReaderWriter.h>

#if LOCIC_LLVM_VERSION >= 305
#include <llvm/IR/DebugInfo.h>
#else
#include <llvm/DebugInfo.h>
#endif

#if LOCIC_LLVM_VERSION >= 306
#include <llvm/IR/DiagnosticInfo.h>
#include <llvm/IR/DiagnosticPrinter.h>
#endif

#if LOCIC_LLVM_VERSION >= 305 && defined(LLVM_DIBUILDER_IN_IR)
#include <llvm/IR/DIBuilder.h>
#else
#include <llvm/DIBuilder.h>
#endif

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>

#if LOCIC_LLVM_VERSION < 306
#include <llvm/ExecutionEngine/JIT.h>
#endif

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
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IRReader/IRReader.h>

#if LOCIC_LLVM_VERSION >= 305 && defined(LLVM_LINKER_IN_LINKER)
#include <llvm/Linker/Linker.h>
#else
#include <llvm/Linker.h>
#endif

#include <llvm/MC/SubtargetFeature.h>

#if LOCIC_LLVM_VERSION >= 307
#include <llvm/IR/LegacyPassManager.h>
#else
#include <llvm/PassManager.h>
#endif

#include <llvm/Support/Dwarf.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#if LOCIC_LLVM_VERSION >= 306
#include <llvm/Target/TargetSubtargetInfo.h>
#endif

#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Scalar.h>

namespace locic {
	
	namespace CodeGen {
		
#if LOCIC_LLVM_VERSION >= 307
		using DICompileUnit = llvm::DICompileUnit*;
		using DIFile = llvm::DIFile*;
		using DILocalVariable = llvm::DILocalVariable*;
		using DINode = llvm::DINode;
		using DIScope = llvm::DIScope*;
		using DISubprogram = llvm::DISubprogram*;
		using DISubroutineType = llvm::DISubroutineType*;
		using DIType = llvm::DIType*;
#else
		using DICompileUnit = llvm::DICompileUnit;
		using DIFile = llvm::DIFile;
		using DILocalVariable = llvm::DIVariable;
		using DINode = llvm::DIDescriptor;
		using DIScope = llvm::DIScope;
		using DISubprogram = llvm::DISubprogram;
		using DISubroutineType = llvm::DICompositeType;
		using DIType = llvm::DIType;
#endif
		
#if LOCIC_LLVM_VERSION >= 306
		using LLVMMetadataValue = llvm::Metadata;
#else
		using LLVMMetadataValue = llvm::Value;
#endif
		
	}
	
}

#endif