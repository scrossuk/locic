#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <fstream>
#include <string>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/DerivedTypes.h>
#include <llvm/GlobalVariable.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <Locic/Map.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

	namespace CodeGen {
	
		class Module {
			public:
				typedef Map<SEM::Function*, llvm::Function*> FunctionMap;
				typedef Map<SEM::Var*, size_t> MemberVarMap;
				typedef Map<SEM::TypeInstance*, llvm::StructType*> TypeMap;
				
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
				
				inline llvm::Module* getLLVMModulePtr() const {
					return module_;
				}
				
				inline FunctionMap& getFunctionMap() {
					return functionMap_;
				}
				
				inline const FunctionMap& getFunctionMap() const {
					return functionMap_;
				}
				
				inline MemberVarMap& getMemberVarMap() {
					return memberVarMap_;
				}
				
				inline const MemberVarMap& getMemberVarMap() const {
					return memberVarMap_;
				}
				
				inline TypeMap& getTypeMap() {
					return typeMap_;
				}
				
				inline const TypeMap& getTypeMap() const {
					return typeMap_;
				}
				
				inline llvm::GlobalVariable* createConstGlobal(const std::string& name,
					llvm::Type* type, llvm::GlobalValue::LinkageTypes linkage,
					llvm::Constant* value) {
					const bool isConstant = true;
					return new llvm::GlobalVariable(getLLVMModule(),
						type, isConstant,
						linkage, value, name);
				}
				
			private:
				llvm::Module* module_;
				TargetInfo targetInfo_;
				FunctionMap functionMap_;
				MemberVarMap memberVarMap_;
				TypeMap typeMap_;
				
		};
		
	}
	
}

#endif
