#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <fstream>
#include <string>
#include <Locic/CodeGen/TargetInfo.hpp>
#include <Locic/CodeGen/TypeMapping.hpp>

namespace Locic {

	namespace CodeGen {
	
		class Module {
			public:
				inline Module(const std::string& name, const TargetInfo& targetInfo)
					: module_(new llvm::Module(name.c_str(), llvm::getGlobalContext())),
					  targetInfo_(targetInfo) {
					module_->setTargetTriple(targetInfo_.getTargetTriple());
				}
				
				inline ~Module() {
					delete module_;
				}
				
				inline void dump() const {
					module_->dump();
				}
				
				inline void dumpToFile(const std::string& fileName) const {
					std::ofstream file(fileName.c_str());
					llvm::raw_os_ostream ostream(file);
					ostream << *(module_);
				}
				
				inline void writeBitCodeToFile(const std::string& fileName) const {
					std::ofstream file(fileName.c_str());
					llvm::raw_os_ostream ostream(file);
					llvm::WriteBitcodeToFile(module_, ostream);
				}
				
				inline const TargetInfo& getTargetInfo() const {
					return targetInfo_;
				}
				
				inline llvm::LLVMContext& getLLVMContext() const {
					return module_->getContext();
				}
				
				inline llvm::Module& getLLVMModule() const {
					return *module_;
				}
				
				inline llvm::Module& getLLVMModule() const {
					return *module_;
				}
				
				inline TypeMapping& getTypeMapping() {
					return typeMapping_;
				}
				
				inline const TypeMapping& getTypeMapping() const {
					return typeMapping_;
				}
				
			private:
				llvm::Module* module_;
				TargetInfo targetInfo_;
				TypeMapping typeMapping_;
				
		}
		
	}
	
}

#endif
