#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>

#include <assert.h>
#include <string>

#include <Locic/Map.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

	namespace CodeGen {
		
		TargetInfo TargetInfo::DefaultTarget(){
			TargetInfo targetInfo;
			
			llvm::InitializeNativeTarget();
			
			targetInfo.triple_ = llvm::sys::getDefaultTargetTriple();
			
			std::string error;
			const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetInfo.getTargetTriple(), error);
			
			assert(target != NULL && "Must find default target.");
					
			assert(target->hasTargetMachine());
			
			std::auto_ptr<llvm::TargetMachine> targetMachine(target->createTargetMachine(targetInfo.getTargetTriple(), "", "", llvm::TargetOptions()));
			const llvm::TargetData* targetData = targetMachine->getTargetData();
			
			assert(targetData != NULL);
			
			targetInfo.isBigEndian_ = targetData->isBigEndian();
			targetInfo.pointerSize_ = targetData->getPointerSize();
			
			clang::CompilerInstance ci;
			ci.createDiagnostics(0, NULL);
			clang::TargetOptions to;
			to.Triple = targetInfo.getTargetTriple();
			clang::TargetInfo* clangTargetInfo = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), to);
			
			targetInfo.primitiveSizes_.insert("size_t",
				clangTargetInfo->getTypeWidth(clangTargetInfo->getSizeType()));
			targetInfo.primitiveSizes_.insert("char", clangTargetInfo->getCharWidth());
			targetInfo.primitiveSizes_.insert("short", clangTargetInfo->getShortWidth());
			targetInfo.primitiveSizes_.insert("int", clangTargetInfo->getIntWidth());
			targetInfo.primitiveSizes_.insert("long", clangTargetInfo->getLongWidth());
			targetInfo.primitiveSizes_.insert("longlong", clangTargetInfo->getLongLongWidth());
			targetInfo.primitiveSizes_.insert("float", clangTargetInfo->getFloatWidth());
			targetInfo.primitiveSizes_.insert("double", clangTargetInfo->getDoubleWidth());
			
			return targetInfo;
		}
		
		TargetInfo::TargetInfo(){ }
		
		TargetInfo::~TargetInfo(){ }
		
		bool TargetInfo::isBigEndian() const{
			return isBigEndian_;
		}
				
		std::string TargetInfo::getTargetTriple() const{
			return triple_;
		}
		
		size_t TargetInfo::getPointerSize() const{
			return pointerSize_;
		}
				
		size_t TargetInfo::getPrimitiveSize(const std::string& name) const{
			return primitiveSizes_.get(name);
		}
		
	}
	
}

