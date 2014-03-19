#include <locic/CodeGen/LLVMIncludes.hpp>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>

#include <assert.h>
#include <string>

#include <locic/Map.hpp>
#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

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
			primitiveSizes_.insert("ssize_t", clangTargetInfo->getTypeWidth(clangTargetInfo->getSizeType()));
			
			primitiveSizes_.insert("null_t", clangTargetInfo->getCharWidth());
			primitiveSizes_.insert("bool", clangTargetInfo->getCharWidth());
			
			primitiveSizes_.insert("char_t", clangTargetInfo->getCharWidth());
			primitiveSizes_.insert("uchar_t", clangTargetInfo->getCharWidth());
			primitiveSizes_.insert("short_t", clangTargetInfo->getShortWidth());
			primitiveSizes_.insert("ushort_t", clangTargetInfo->getShortWidth());
			primitiveSizes_.insert("int_t", clangTargetInfo->getIntWidth());
			primitiveSizes_.insert("uint_t", clangTargetInfo->getIntWidth());
			primitiveSizes_.insert("long_t", clangTargetInfo->getLongWidth());
			primitiveSizes_.insert("ulong_t", clangTargetInfo->getLongWidth());
			primitiveSizes_.insert("longlong_t", clangTargetInfo->getLongLongWidth());
			primitiveSizes_.insert("ulonglong_t", clangTargetInfo->getLongLongWidth());
			primitiveSizes_.insert("integer_literal_t", clangTargetInfo->getLongLongWidth());
			
			primitiveSizes_.insert("float_t", clangTargetInfo->getFloatWidth());
			primitiveSizes_.insert("double_t", clangTargetInfo->getDoubleWidth());
			primitiveSizes_.insert("longdouble_t", clangTargetInfo->getLongDoubleWidth());
			primitiveSizes_.insert("float_literal_t", clangTargetInfo->getLongDoubleWidth());
			
			primitiveSizes_.insert("ptr", clangTargetInfo->getPointerWidth(0));
			
			primitiveSizes_.insert("int8_t", 8);
			primitiveSizes_.insert("uint8_t", 8);
			primitiveSizes_.insert("int16_t", 16);
			primitiveSizes_.insert("uint16_t", 16);
			primitiveSizes_.insert("int32_t", 32);
			primitiveSizes_.insert("uint32_t", 32);
			primitiveSizes_.insert("int64_t", 64);
			primitiveSizes_.insert("uint64_t", 64);
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
		
		size_t TargetInfo::getPrimitiveAlign(const std::string& name) const {
			// TODO
			return getPrimitiveSize(name);
		}
		
	}
	
}

