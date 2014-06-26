#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Context.hpp>

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
	
		typedef std::pair<llvm_abi::Type*, llvm::Type*> TypePair;
		
		enum CompareResult {
			COMPARE_EQUAL,
			COMPARE_LESS,
			COMPARE_MORE
		};
		
		enum StandardTypeKind {
			TemplateGeneratorType,
			TypeInfoType
		};
		
		CompareResult compareTypes(SEM::Type* const first, SEM::Type* const second);
		
		inline bool isTypeLessThan(SEM::Type* first, SEM::Type* second) {
			return compareTypes(first, second) == COMPARE_LESS;
		}
		
		typedef std::unordered_map<TemplateBuilder*, llvm::GlobalAlias*> BitsRequiredGlobalMap;
		typedef std::unordered_map<SEM::TypeInstance*, llvm::Function*> DestructorMap;
		typedef Map<std::string, llvm::Function*> FunctionMap;
		typedef std::unordered_map<SEM::Function*, llvm::Function*> FunctionDeclMap;
		typedef std::unordered_map<SEM::TypeInstance*, llvm::Function*> MemberOffsetFunctionMap;
		typedef Map<SEM::Var*, size_t> MemberVarMap;
		typedef std::unordered_map<std::string, PrimitiveKind> PrimitiveMap;
		typedef std::map<StandardTypeKind, TypePair> StandardTypeMap;
		typedef std::unordered_map<SEM::TypeInstance*, TemplateBuilder> TemplateBuilderMap;
		typedef std::map<SEM::Type*, llvm::Function*, bool(*)(SEM::Type*, SEM::Type*)> TemplateRootFunctionMap;
		typedef Map<SEM::TemplateVar*, SEM::Type*> TemplateVarMap;
		typedef Map<std::string, llvm::StructType*> TypeMap;
		typedef std::unordered_map<SEM::TypeInstance*, llvm::StructType*> TypeInstanceMap;
		
		class Module {
			public:
				Module(const std::string& name, const TargetInfo& targetInfo, Debug::Module& pDebugModule);
				
				void dump() const;
				
				void dumpToFile(const std::string& fileName) const;
				
				void writeBitCodeToFile(const std::string& fileName) const;
				
				const TargetInfo& getTargetInfo() const;
				
				llvm_abi::ABI& abi();
				
				const llvm_abi::ABI& abi() const;
				
				llvm_abi::Context& abiContext();
				
				llvm::LLVMContext& getLLVMContext() const;
				
				llvm::Module& getLLVMModule() const;
				
				llvm::Module* getLLVMModulePtr() const;
				
				BitsRequiredGlobalMap& bitsRequiredGlobalMap();
				
				DestructorMap& getDestructorMap();
				
				FunctionMap& getFunctionMap();
				
				const FunctionMap& getFunctionMap() const;
				
				FunctionDeclMap& getFunctionDeclMap();
				
				MemberOffsetFunctionMap& memberOffsetFunctionMap();
				
				MemberVarMap& getMemberVarMap();
				
				const MemberVarMap& getMemberVarMap() const;
				
				StandardTypeMap& standardTypeMap();
				
				TemplateBuilder& typeTemplateBuilder(SEM::TypeInstance* typeInstance);
				
				TemplateRootFunctionMap& templateRootFunctionMap();
				
				TypeMap& getTypeMap();
				
				const TypeMap& getTypeMap() const;
				
				TypeInstanceMap& typeInstanceMap();
				
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
				llvm_abi::Context abiContext_;
				
				BitsRequiredGlobalMap bitsRequiredGlobalMap_;
				DestructorMap destructorMap_;
				FunctionMap functionMap_;
				FunctionDeclMap functionDeclMap_;
				MemberOffsetFunctionMap memberOffsetFunctionMap_;
				MemberVarMap memberVarMap_;
				PrimitiveMap primitiveMap_;
				StandardTypeMap standardTypeMap_;
				TemplateBuilderMap templateBuilderMap_;
				TemplateRootFunctionMap templateRootFunctionMap_;
				TypeMap typeMap_;
				TypeInstanceMap typeInstanceMap_;
				DebugBuilder debugBuilder_;
				Debug::Module& debugModule_;
				
		};
		
	}
	
}

#endif
