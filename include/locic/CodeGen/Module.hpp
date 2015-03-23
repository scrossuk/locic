#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

#include <llvm-abi/ABI.hpp>

#include <locic/BuildOptions.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TemplateBuilder.hpp>
#include <locic/Support/FastMap.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace Debug {
		
		class Module;
		
	}
	
	namespace SEM {
		
		class Function;
		class Type;
		class TypeInstance;
		class Var;
		
	}
	
	namespace CodeGen {
	
		typedef std::pair<llvm_abi::Type*, llvm::Type*> TypePair;
		
		enum CompareResult {
			COMPARE_EQUAL,
			COMPARE_LESS,
			COMPARE_MORE
		};
		
		enum AttributeKind {
			AttributeVirtualCallStub
		};
		
		enum StandardTypeKind {
			TemplateGeneratorType,
			TypeInfoType
		};
		
		template <typename Key, typename Value>
		struct hashPair {
			std::size_t operator()(const std::pair<Key, Value>& pair) const {
				std::size_t seed = 0;
				std::hash<Key> keyHashFn;
				boost::hash_combine(seed, keyHashFn(pair.first));
				std::hash<Value> valueHashFn;
				boost::hash_combine(seed, valueHashFn(pair.second));
				return seed;
			}
		};
		
		typedef FastMap<AttributeKind, llvm::AttributeSet> AttributeMap;
		typedef std::unordered_map<TemplateBuilder*, llvm::GlobalAlias*> BitsRequiredGlobalMap;
		typedef std::unordered_map<const SEM::TypeInstance*, llvm::Function*> DestructorMap;
		typedef FastMap<String, llvm::Function*> FunctionMap;
		typedef FastMap<std::pair<llvm::Function*, llvm::FunctionType*>, llvm::Function*> FunctionPtrStubMap;
		typedef std::unordered_map<SEM::Function*, llvm::Function*> FunctionDeclMap;
		typedef std::unordered_map<std::pair<String, Name>, String, hashPair<String, Name>> MangledNameMap;
		typedef std::unordered_map<const SEM::TypeInstance*, llvm::Function*> MemberOffsetFunctionMap;
		typedef FastMap<const SEM::Var*, size_t> MemberVarMap;
		typedef std::unordered_map<const SEM::TypeInstance*, llvm::Function*> MoveFunctionMap;
		typedef std::unordered_map<String, PrimitiveKind> PrimitiveMap;
		typedef FastMap<StandardTypeKind, TypePair> StandardTypeMap;
		typedef FastMap<TemplatedObject, TemplateBuilder> TemplateBuilderMap;
		typedef FastMap<TemplateInst, llvm::Function*> TemplateRootFunctionMap;
		typedef FastMap<SEM::TemplateVar*, const SEM::Type*> TemplateVarMap;
		typedef FastMap<String, llvm::StructType*> TypeMap;
		typedef std::unordered_map<const SEM::TypeInstance*, llvm::StructType*> TypeInstanceMap;
		
		class InternalContext;
		
		class Module {
			public:
				Module(InternalContext& context, const std::string& name, Debug::Module& pDebugModule, const BuildOptions& pBuildOptions);
				
				String getCString(const char* cString) const;
				
				String getString(std::string stringValue) const;
				
				void dump() const;
				
				void dumpToFile(const std::string& fileName) const;
				
				void writeBitCodeToFile(const std::string& fileName) const;
				
				llvm_abi::ABI& abi();
				
				const llvm_abi::ABI& abi() const;
				
				llvm_abi::Context& abiContext();
				
				llvm::LLVMContext& getLLVMContext() const;
				
				std::unique_ptr<llvm::Module> releaseLLVMModule();
				
				llvm::Module& getLLVMModule() const;
				
				llvm::Module* getLLVMModulePtr() const;
				
				AttributeMap& attributeMap();
				
				BitsRequiredGlobalMap& bitsRequiredGlobalMap();
				
				DestructorMap& getDestructorMap();
				
				FunctionMap& getFunctionMap();
				
				FunctionDeclMap& getFunctionDeclMap();
				
				FunctionPtrStubMap& functionPtrStubMap();
				
				MangledNameMap& mangledNameMap();
				
				MemberOffsetFunctionMap& memberOffsetFunctionMap();
				
				MemberVarMap& getMemberVarMap();
				
				const MemberVarMap& getMemberVarMap() const;
				
				MoveFunctionMap& getMoveFunctionMap();
				
				StandardTypeMap& standardTypeMap();
				
				TemplateBuilder& templateBuilder(TemplatedObject templatedObject);
				
				TemplateRootFunctionMap& templateRootFunctionMap();
				
				TypeMap& getTypeMap();
				
				const TypeMap& getTypeMap() const;
				
				TypeInstanceMap& typeInstanceMap();
				
				llvm::GlobalVariable* createConstGlobal(const String& name,
					llvm::Type* type, llvm::GlobalValue::LinkageTypes linkage,
					llvm::Constant* value = nullptr);
				
				DebugBuilder& debugBuilder();
				
				Debug::Module& debugModule();
				
				const BuildOptions& buildOptions() const;
				
				PrimitiveKind primitiveKind(const String& name) const;
				
				void verify() const;
				
			private:
				InternalContext& context_;
				std::unique_ptr<llvm::Module> module_;
				std::unique_ptr<llvm_abi::ABI> abi_;
				AttributeMap attributeMap_;
				BitsRequiredGlobalMap bitsRequiredGlobalMap_;
				DestructorMap destructorMap_;
				FunctionMap functionMap_;
				FunctionDeclMap functionDeclMap_;
				FunctionPtrStubMap functionPtrStubMap_;
				MangledNameMap mangledNameMap_;
				MemberOffsetFunctionMap memberOffsetFunctionMap_;
				MemberVarMap memberVarMap_;
				MoveFunctionMap moveFunctionMap_;
				PrimitiveMap primitiveMap_;
				StandardTypeMap standardTypeMap_;
				TemplateBuilderMap templateBuilderMap_;
				TemplateRootFunctionMap templateRootFunctionMap_;
				TypeMap typeMap_;
				TypeInstanceMap typeInstanceMap_;
				DebugBuilder debugBuilder_;
				Debug::Module& debugModule_;
				BuildOptions buildOptions_;
				
		};
		
	}
	
}

#endif
