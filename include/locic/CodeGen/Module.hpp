#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <fstream>
#include <string>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

	namespace CodeGen {
	
		class Module {
			public:
				typedef Map<std::string, llvm::Function*> FunctionMap;
				typedef Map<SEM::Var*, size_t> MemberVarMap;
				typedef Map<SEM::TemplateVar*, SEM::Type*> TemplateVarMap;
				typedef Map<std::string, llvm::StructType*> TypeMap;
				
				inline Module(const std::string& name, const TargetInfo& targetInfo)
					: module_(new llvm::Module(name.c_str(), llvm::getGlobalContext())),
					  targetInfo_(targetInfo), debugBuilder_(*this) {
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
					templateVarMapStack_.push_back(&templateVarMap);
				}
				
				inline void popTemplateVarMap() {
					templateVarMapStack_.pop_back();
				}
				
				inline SEM::Type* resolveType(SEM::Type* type) const {
					for(size_t i = 0; i < templateVarMapStack_.size(); i++) {
						assert(type != NULL);
						if (!type->isTemplateVar()) {
							return type;
						}
						const TemplateVarMap* map = templateVarMapStack_.at(templateVarMapStack_.size() - i - 1);
						assert(map != NULL);
						const Optional<SEM::Type*> result = map->tryGet(type->getTemplateVar());
						if (result.hasValue()) {
							LOG(LOG_INFO, "Resolved %s -> %s.",
								type->toString().c_str(),
								result.getValue()->toString().c_str());
							type = result.getValue();
						}
					}
					
					if (!type->isTemplateVar()) {
						return type;
					}
					
					LOG(LOG_INFO, "Failed to resolve type %s.",
						type->toString().c_str());
					
					assert(false && "Failed to resolve type.");
					
					return NULL;
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
				
				inline DebugBuilder& debugBuilder() {
					return debugBuilder_;
				}
				
			private:
				llvm::Module* module_;
				TargetInfo targetInfo_;
				FunctionMap functionMap_;
				MemberVarMap memberVarMap_;
				std::vector<const TemplateVarMap*> templateVarMapStack_;
				TypeMap typeMap_;
				DebugBuilder debugBuilder_;
				
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
