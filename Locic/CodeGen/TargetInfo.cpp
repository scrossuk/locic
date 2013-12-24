#include <Locic/CodeGen/LLVMIncludes.hpp>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>

#include <assert.h>
#include <string>

#include <Locic/Map.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

	namespace CodeGen {
	
		TargetInfo TargetInfo::DefaultTarget() {
			llvm::InitializeNativeTarget();
			return TargetInfo(llvm::sys::getDefaultTargetTriple());
		}
		
		TargetInfo::TargetInfo(const std::string& triple)
			: triple_(triple) {
			
			std::string error;
			const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple, error);
			
			assert(target != NULL && "Must find target.");
			
			assert(target->hasTargetMachine());
			
			std::auto_ptr<llvm::TargetMachine> targetMachine(target->createTargetMachine(triple, "", "", llvm::TargetOptions()));
			const llvm::DataLayout* dataLayout = targetMachine->getDataLayout();
			
			assert(dataLayout != NULL);
			
			isBigEndian_ = dataLayout->isBigEndian();
			pointerSize_ = dataLayout->getPointerSize();
			
			clang::CompilerInstance ci;
			ci.createDiagnostics(NULL, false);
			clang::TargetOptions targetOptions;
			targetOptions.Triple = triple;
			clang::TargetInfo* clangTargetInfo = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), &targetOptions);
			
			primitiveSizes_.insert("size_t", clangTargetInfo->getTypeWidth(clangTargetInfo->getSizeType()));
			primitiveSizes_.insert("null_t", clangTargetInfo->getCharWidth());
			primitiveSizes_.insert("bool", clangTargetInfo->getCharWidth());
			primitiveSizes_.insert("char", clangTargetInfo->getCharWidth());
			primitiveSizes_.insert("short", clangTargetInfo->getShortWidth());
			primitiveSizes_.insert("int", clangTargetInfo->getIntWidth());
			primitiveSizes_.insert("long", clangTargetInfo->getLongWidth());
			primitiveSizes_.insert("longlong", clangTargetInfo->getLongLongWidth());
			primitiveSizes_.insert("float", clangTargetInfo->getFloatWidth());
			primitiveSizes_.insert("double", clangTargetInfo->getDoubleWidth());
			primitiveSizes_.insert("ptr", clangTargetInfo->getPointerWidth(0));
		}
		
		TargetInfo::~TargetInfo() { }
		
		bool TargetInfo::isBigEndian() const {
			return isBigEndian_;
		}
		
		std::string TargetInfo::getTargetTriple() const {
			return triple_;
		}
		
		size_t TargetInfo::getPointerSize() const {
			return pointerSize_;
		}
		
		size_t TargetInfo::getPrimitiveSize(const std::string& name) const {
			if (!primitiveSizes_.has(name)) {
				LOG(LOG_CRITICAL, "Can't find primitive type '%s'.", name.c_str());
			}
			return primitiveSizes_.get(name);
		}
		
	}
	
}

