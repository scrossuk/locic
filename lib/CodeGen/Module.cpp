#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>

#include <locic/Debug.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

	namespace CodeGen {
	
		Module::Module(const std::string& name, const TargetInfo& targetInfo, Debug::Module& pDebugModule)
			: module_(new llvm::Module(name.c_str(), llvm::getGlobalContext())),
			  targetInfo_(targetInfo), abi_(llvm_abi::createABI(module_->getContext(), targetInfo_.getTargetTriple())),
			  debugBuilder_(*this), debugModule_(pDebugModule) {
			module_->setDataLayout(abi_->dataLayout().getStringRepresentation());
			module_->setTargetTriple(targetInfo_.getTargetTriple());
		}
		
		void Module::dump() const {
			module_->dump();
		}
		
		void Module::dumpToFile(const std::string& fileName) const {
			std::ofstream file(fileName.c_str());
			llvm::raw_os_ostream ostream(file);
			ostream << *(module_);
		}
		
		void Module::writeBitCodeToFile(const std::string& fileName) const {
			std::ofstream file(fileName.c_str());
			llvm::raw_os_ostream ostream(file);
			llvm::WriteBitcodeToFile(module_.get(), ostream);
		}
		
		const TargetInfo& Module::getTargetInfo() const {
			return targetInfo_;
		}
		
		llvm_abi::ABI& Module::abi() {
			return *abi_;
		}
		
		const llvm_abi::ABI& Module::abi() const {
			return *abi_;
		}
		
		llvm::LLVMContext& Module::getLLVMContext() const {
			return module_->getContext();
		}
		
		llvm::Module& Module::getLLVMModule() const {
			return *module_;
		}
		
		llvm::Module* Module::getLLVMModulePtr() const {
			return module_.get();
		}
		
		FunctionMap& Module::getFunctionMap() {
			return functionMap_;
		}
		
		const FunctionMap& Module::getFunctionMap() const {
			return functionMap_;
		}
		
		MemberVarMap& Module::getMemberVarMap() {
			return memberVarMap_;
		}
		
		const MemberVarMap& Module::getMemberVarMap() const {
			return memberVarMap_;
		}
		
		void Module::pushTemplateVarMap(const TemplateVarMap& templateVarMap) {
			templateVarMapStack_.push_back(&templateVarMap);
		}
		
		void Module::popTemplateVarMap() {
			templateVarMapStack_.pop_back();
		}
		
		SEM::Type* Module::resolveType(SEM::Type* type) const {
			for (size_t i = 0; i < templateVarMapStack_.size(); i++) {
				if (!type->isTemplateVar()) {
					return type;
				}
				
				const auto map = templateVarMapStack_.at(templateVarMapStack_.size() - i - 1);
				const auto result = map->tryGet(type->getTemplateVar());
				
				if (result.hasValue()) {
					type = result.getValue();
				}
			}
			
			if (!type->isTemplateVar()) {
				return type;
			}
			
			assert(false && "Failed to resolve type.");
			
			return nullptr;
		}
		
		TypeMap& Module::getTypeMap() {
			return typeMap_;
		}
		
		const TypeMap& Module::getTypeMap() const {
			return typeMap_;
		}
		
		llvm::GlobalVariable* Module::createConstGlobal(const std::string& name,
				llvm::Type* type, llvm::GlobalValue::LinkageTypes linkage,
				llvm::Constant* value) {
			const bool isConstant = true;
			return new llvm::GlobalVariable(getLLVMModule(), type, isConstant, linkage, value, name);
		}
		
		DebugBuilder& Module::debugBuilder() {
			return debugBuilder_;
		}
		
		Debug::Module& Module::debugModule() {
			return debugModule_;
		}
		
	}
	
}

