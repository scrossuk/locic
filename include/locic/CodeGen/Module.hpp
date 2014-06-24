#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <llvm-abi/ABI.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Debug.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TargetInfo.hpp>
#include <locic/CodeGen/TemplateBuilder.hpp>

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
		typedef std::unordered_map<std::string, PrimitiveKind> PrimitiveMap;
		typedef std::map<SEM::TypeInstance*, TemplateBuilder> TemplateBuilderMap;
		typedef std::map<TemplateBuilder*, llvm::Function*> TemplateBuilderFunctionMap;
		typedef Map<SEM::Type*, llvm::Function*> TemplateGeneratorMap;
		typedef Map<SEM::TemplateVar*, SEM::Type*> TemplateVarMap;
		typedef Map<std::string, llvm::StructType*> TypeMap;
		
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
				
				TemplateBuilder& typeTemplateBuilder(SEM::TypeInstance* typeInstance);
				
				TemplateBuilderFunctionMap& templateBuilderFunctionMap();
				
				TemplateGeneratorMap& getTemplateGeneratorMap();
				
				const TemplateGeneratorMap& getTemplateGeneratorMap() const;
				
				TypeMap& getTypeMap();
				
				const TypeMap& getTypeMap() const;
				
				llvm::GlobalVariable* createConstGlobal(const std::string& name,
					llvm::Type* type, llvm::GlobalValue::LinkageTypes linkage,
					llvm::Constant* value = nullptr);
				
				DebugBuilder& debugBuilder();
				
				Debug::Module& debugModule();
				
				PrimitiveKind primitiveKind(const std::string& name) const;
				
			private:
				std::unique_ptr<llvm::Module> module_;
				TargetInfo targetInfo_;
				std::unique_ptr<llvm_abi::ABI> abi_;
				FunctionMap functionMap_;
				MemberVarMap memberVarMap_;
				PrimitiveMap primitiveMap_;
				TemplateBuilderMap templateBuilderMap_;
				TemplateBuilderFunctionMap templateBuilderFunctionMap_;
				TemplateGeneratorMap templateGeneratorMap_;
				TypeMap typeMap_;
				DebugBuilder debugBuilder_;
				Debug::Module& debugModule_;
				
		};
		
	}
	
}

#endif
