#ifndef LOCIC_CODEGEN_MODULE_HPP
#define LOCIC_CODEGEN_MODULE_HPP

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

#include <llvm-abi/ABI.hpp>

#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/BuildOptions.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/PrimitiveMap.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TemplateBuilder.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	class PrimitiveID;
	
	namespace AST {
		
		class Function;
		class Type;
		class TypeInstance;
		
	}
	
	namespace Debug {
		
		class Module;
		
	}
	
	namespace CodeGen {
		
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
		typedef std::unordered_map<const AST::TypeInstance*, llvm::Function*> DestructorMap;
		typedef FastMap<String, llvm::Function*> FunctionMap;
		typedef FastMap<std::pair<llvm::Function*, llvm::FunctionType*>, llvm::Function*> FunctionPtrStubMap;
		typedef std::unordered_map<const AST::Function*, llvm::Function*> FunctionDeclMap;
		typedef std::unordered_map<std::pair<String, Name>, String, hashPair<String, Name>> MangledNameMap;
		typedef std::unordered_map<const AST::TypeInstance*, llvm::Function*> MemberOffsetFunctionMap;
		typedef std::unordered_map<const AST::TypeInstance*, llvm::Function*> MoveFunctionMap;
		typedef FastMap<StandardTypeKind, llvm_abi::Type> StandardTypeMap;
		typedef FastMap<TemplatedObject, TemplateBuilder> TemplateBuilderMap;
		typedef FastMap<TemplateInst, llvm::Function*> TemplateRootFunctionMap;
		typedef FastMap<String, llvm::StructType*> TypeMap;
		typedef std::unordered_map<const AST::TypeInstance*, llvm::StructType*> TypeInstanceMap;
		
		class InternalContext;
		class Primitive;
		
		class Module {
			public:
				Module(InternalContext& context, const std::string& name, Debug::Module& pDebugModule, const BuildOptions& pBuildOptions);
				
				InternalContext& context();
				
				String getCString(const char* cString) const;
				
				String getString(std::string stringValue) const;
				
				void dump() const;
				
				void dumpToFile(const std::string& fileName) const;
				
				void writeBitCodeToFile(const std::string& fileName) const;
				
				llvm_abi::ABI& abi();
				
				const llvm_abi::ABI& abi() const;
				
				const llvm_abi::TypeBuilder& abiTypeBuilder() const;
				
				llvm::Type* getLLVMType(llvm_abi::Type type) const;
				
				VirtualCallABI& virtualCallABI();
				
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
				
				MoveFunctionMap& getMoveFunctionMap();
				
				const Primitive& getPrimitive(const AST::TypeInstance& typeInstance) const;
				
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
				
				PrimitiveID primitiveID(const String& name) const;
				
				ASTFunctionGenerator& astFunctionGenerator();
				
				void verify() const;
				
			private:
				InternalContext& context_;
				std::unique_ptr<llvm::Module> module_;
				std::unique_ptr<llvm_abi::ABI> abi_;
				std::unique_ptr<VirtualCallABI> virtualCallABI_;
				AttributeMap attributeMap_;
				BitsRequiredGlobalMap bitsRequiredGlobalMap_;
				DestructorMap destructorMap_;
				FunctionMap functionMap_;
				FunctionDeclMap functionDeclMap_;
				FunctionPtrStubMap functionPtrStubMap_;
				MangledNameMap mangledNameMap_;
				MemberOffsetFunctionMap memberOffsetFunctionMap_;
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
				ASTFunctionGenerator astFunctionGenerator_;
				
		};
		
	}
	
}

#endif
