#ifndef LOCIC_CODEGEN_LLVMINCLUDES_HPP
#define LOCIC_CODEGEN_LLVMINCLUDES_HPP

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Analysis/Passes.h>

#ifdef LLVM_3_5
#include <llvm/IR/Verifier.h>
#else
#include <llvm/Analysis/Verifier.h>
#endif

#include <llvm/Bitcode/ReaderWriter.h>

#ifdef LLVM_3_5
#include <llvm/DebugInfo/DIContext.h>
#else
#include <llvm/DebugInfo.h>
#endif

#if defined(LLVM_3_5) && defined(LLVM_DIBUILDER_IN_IR)
#include <llvm/IR/DIBuilder.h>
#else
#include <llvm/DIBuilder.h>
#endif

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
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

#if defined(LLVM_3_5) && defined(LLVM_LINKER_IN_LINKER)
#include <llvm/Linker/Linker.h>
#else
#include <llvm/Linker.h>
#endif

#include <llvm/PassManager.h>
#include <llvm/Support/Dwarf.h>
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

#endif