#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Debug.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

	namespace CodeGen {
	
		enum CompareResult {
			COMPARE_EQUAL,
			COMPARE_LESS,
			COMPARE_MORE
		};
		
		CompareResult compareTypes(SEM::Type* const first, SEM::Type* const second);
		
		inline bool isTypeLessThan(SEM::Type* first, SEM::Type* second) {
			return compareTypes(first, second) == COMPARE_LESS;
		}
		
		typedef Map<std::string, llvm::Function*> FunctionMap;
		typedef Map<SEM::Var*, size_t> MemberVarMap;
		typedef Map<SEM::TemplateVar*, SEM::Type*> TemplateVarMap;
		typedef Map<std::string, llvm::StructType*> TypeMap;
		typedef Map<SEM::Type*, llvm::Function*> TemplateGeneratorMap;
		
		class Module {
			public:
				Module(const std::string& name, const TargetInfo& targetInfo, Debug::Module& pDebugModule);
				
				void dump() const;
				
				void dumpToFile(const std::string& fileName) const;
				
				void writeBitCodeToFile(const std::string& fileName) const;
				
				const TargetInfo& getTargetInfo() const;
				
				llvm_abi::ABI& abi();
				
				const llvm_abi::ABI& abi() const;
				
				llvm::LLVMContext& getLLVMContext() const;
				
				llvm::Module& getLLVMModule() const;
				
				llvm::Module* getLLVMModulePtr() const;
				
				FunctionMap& getFunctionMap();
				
				const FunctionMap& getFunctionMap() const;
				
				MemberVarMap& getMemberVarMap();
				
				const MemberVarMap& getMemberVarMap() const;
				
				TypeMap& getTypeMap();
				
				const TypeMap& getTypeMap() const;
				
				TemplateGeneratorMap& getTemplateGeneratorMap();
				
				const TemplateGeneratorMap& getTemplateGeneratorMap() const;
				
				llvm::GlobalVariable* createConstGlobal(const std::string& name,
					llvm::Type* type, llvm::GlobalValue::LinkageTypes linkage,
					llvm::Constant* value = nullptr);
				
				DebugBuilder& debugBuilder();
				
				Debug::Module& debugModule();
				
			private:
				std::unique_ptr<llvm::Module> module_;
				TargetInfo targetInfo_;
				std::unique_ptr<llvm_abi::ABI> abi_;
				FunctionMap functionMap_;
				MemberVarMap memberVarMap_;
				TypeMap typeMap_;
				TemplateGeneratorMap templateGeneratorMap_;
				DebugBuilder debugBuilder_;
				Debug::Module& debugModule_;
				
		};
		
	}
	
}

#endif
