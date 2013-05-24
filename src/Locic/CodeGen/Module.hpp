#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <fstream>
#include <stack>
#include <string>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/DerivedTypes.h>
#include <llvm/GlobalVariable.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

	namespace CodeGen {
	
		class Module {
			public:
				typedef Map<std::string, llvm::Function*> FunctionMap;
				typedef Map<SEM::Var*, size_t> MemberVarMap;
				typedef Map<SEM::TemplateVar*, SEM::Type*> TemplateVarMap;
				typedef Map<std::string, llvm::StructType*> TypeMap;
				
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
				
				inline void pushTemplateVarMap(const TemplateVarMap& templateVarMap) {
					templateVarMapStack_.push(&templateVarMap);
				}
				
				inline void popTemplateVarMap() {
					templateVarMapStack_.pop();
				}
				
				inline SEM::Type* getTemplateVarValue(SEM::TemplateVar* templateVar) const {
					assert(templateVar != NULL);
					return templateVarMapStack_.top()->get(templateVar);
				}
				
				inline TypeMap& getTypeMap() {
					return typeMap_;
				}
				
				inline const TypeMap& getTypeMap() const {
					return typeMap_;
				}
				
				inline llvm::GlobalVariable* createConstGlobal(const std::string& name,
					llvm::Type* type, llvm::GlobalValue::LinkageTypes linkage,
					llvm::Constant* value = NULL) {
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
				std::stack<const TemplateVarMap*> templateVarMapStack_;
				TypeMap typeMap_;
				
		};
		
		class TemplateVarMapStackEntry {
			public:
				inline TemplateVarMapStackEntry(Module& module, const Module::TemplateVarMap& templateVarMap)
					: module_(module) {
						module_.pushTemplateVarMap(templateVarMap);
					}
				
				inline ~TemplateVarMapStackEntry() {
					module_.popTemplateVarMap();
				}
			
			private:
				Module& module_;
				
		};
		
	}
	
}

#endif
