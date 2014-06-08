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
		
		TemplateBuilder& Module::typeTemplateBuilder(SEM::TypeInstance* typeInstance) {
			return templateBuilderMap_[typeInstance];
		}
		
		TemplateBuilderFunctionMap& Module::templateBuilderFunctionMap() {
			return templateBuilderFunctionMap_;
		}
		
		TemplateGeneratorMap& Module::getTemplateGeneratorMap() {
			return templateGeneratorMap_;
		}
		
		const TemplateGeneratorMap& Module::getTemplateGeneratorMap() const {
			return templateGeneratorMap_;
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
		
		CompareResult compareTypes(SEM::Type* const first, SEM::Type* const second) {
			if (first == second) {
				return COMPARE_EQUAL;
			}
			
			if (first->kind() != second->kind()) {
				return first->kind() < second->kind() ? COMPARE_LESS : COMPARE_MORE;
			}
			
			if (first->isConst() != second->isConst()) {
				return first->isConst() && !second->isConst() ? COMPARE_LESS : COMPARE_MORE;
			}
			
			if (first->isLval() != second->isLval()) {
				return first->isLval() && !second->isLval() ? COMPARE_LESS : COMPARE_MORE;
			}
			
			if (first->isRef() != second->isRef()) {
				return first->isRef() && !second->isRef() ? COMPARE_LESS : COMPARE_MORE;
			}
			
			if (first->isLval()) {
				const auto result = compareTypes(first->lvalTarget(), second->lvalTarget());
				if (result != COMPARE_EQUAL) {
					return result;
				}
			}
			
			if (first->isRef()) {
				const auto result = compareTypes(first->refTarget(), second->refTarget());
				if (result != COMPARE_EQUAL) {
					return result;
				}
			}
			
			switch (first->kind()) {
				case SEM::Type::VOID:
					return COMPARE_EQUAL;
				
				case SEM::Type::OBJECT: {
					if (first->getObjectType() != second->getObjectType()) {
						return first->getObjectType() < second->getObjectType() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					if (first->templateArguments().size() != second->templateArguments().size()) {
						return first->templateArguments().size() < second->templateArguments().size() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					for (size_t i = 0; i < first->templateArguments().size(); i++) {
						const auto result = compareTypes(first->templateArguments().at(i), second->templateArguments().at(i));
						if (result != COMPARE_EQUAL) {
							return result;
						}
					}
					
					return COMPARE_EQUAL;
				}
				
				case SEM::Type::FUNCTION: {
					if (first->isFunctionVarArg() != second->isFunctionVarArg()) {
						return first->isFunctionVarArg() && !second->isFunctionVarArg() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					if (first->isFunctionMethod() != second->isFunctionMethod()) {
						return first->isFunctionMethod() && !second->isFunctionMethod() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					if (first->isFunctionTemplatedMethod() != second->isFunctionTemplatedMethod()) {
						return first->isFunctionTemplatedMethod() && !second->isFunctionTemplatedMethod() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					if (first->isFunctionNoExcept() != second->isFunctionNoExcept()) {
						return first->isFunctionNoExcept() && !second->isFunctionNoExcept() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					{
						const auto result = compareTypes(first->getFunctionReturnType(), second->getFunctionReturnType());
						if (result != COMPARE_EQUAL) {
							return result;
						}
					}
					
					const auto& firstList = first->getFunctionParameterTypes();
					const auto& secondList = second->getFunctionParameterTypes();
					
					if (firstList.size() != secondList.size()) {
						return firstList.size() < secondList.size() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					for (size_t i = 0; i < firstList.size(); i++) {
						const auto result = compareTypes(firstList.at(i), secondList.at(i));
						if (result != COMPARE_EQUAL) {
							return result;
						}
					}
					
					return COMPARE_EQUAL;
				}
				
				case SEM::Type::METHOD: {
					return compareTypes(first->getMethodFunctionType(), second->getMethodFunctionType());
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					return compareTypes(first->getInterfaceMethodFunctionType(), second->getInterfaceMethodFunctionType());
				}
				
				case SEM::Type::TEMPLATEVAR: {
					if (first->getTemplateVar() != second->getTemplateVar()) {
						return first->getTemplateVar() < second->getTemplateVar() ? COMPARE_LESS : COMPARE_MORE;
					}
					
					return COMPARE_EQUAL;
				}
				
				default: {
					llvm_unreachable("Unknown type enum in comparison.");
				}
			}
		}
		
	}
	
}

