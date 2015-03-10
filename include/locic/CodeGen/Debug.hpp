#ifndef LOCIC_CODEGEN_DEBUG_HPP
#define LOCIC_CODEGEN_DEBUG_HPP

#include <string>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/Debug.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/SEM.hpp>

namespace locic {

	namespace CodeGen {
		
		struct DebugCompileUnit {
			std::string compilerName;
			std::string directoryName;
			std::string fileName;
			std::string flags;
		};
		
		class Function;
		class Module;
		
#if defined(LLVM_3_6)
		using LLVMMetadataValue = llvm::Metadata;
#else
		using LLVMMetadataValue = llvm::Value;
#endif
		
		class DebugBuilder {
			public:
				DebugBuilder(Module& module);
				~DebugBuilder();
				
				void finalize();
				
				llvm::DICompileUnit createCompileUnit(const DebugCompileUnit& compileUnit);
				
				llvm::DICompileUnit compileUnit() const;
				
				llvm::DIFile createFile(const std::string& path);
				
				llvm::DISubprogram createFunction(llvm::DIFile file, unsigned int lineNumber,
					bool isInternal, bool isDefinition, const Name& name,
					llvm::DIType functionType, llvm::Function* function);
				
				llvm::DIVariable createVar(llvm::DIDescriptor scope, bool isParam, const String& name, llvm::DIFile file, unsigned lineNumber, llvm::DIType type);
				
				llvm::DIType createVoidType();
				
				llvm::DIType createNullType();
				
				llvm::DIType createReferenceType(llvm::DIType type);
				
				llvm::DIType createPointerType(llvm::DIType type);
				
				llvm::DIType createIntType(const String& name);
				
				llvm::DIType createObjectType(llvm::DIFile file, unsigned int lineNumber, const Name& name);
				
				llvm::DIType createFunctionType(llvm::DIFile file, const std::vector<LLVMMetadataValue*>& parameters);
				
				llvm::Instruction* insertVariableDeclare(Function& function, llvm::DIVariable variable, llvm::Value* varValue);
				
			private:
				Module& module_;
				llvm::DIBuilder builder_;
			
		};
		
		std::pair<std::string, std::string> splitPath(const std::string& path);
		
		llvm::DISubprogram genDebugFunction(Module& module, const Debug::FunctionInfo& functionInfo, llvm::DIType functionType, llvm::Function* function, bool isInternal);
		
		llvm::Instruction* genDebugVar(Function& function, const Debug::VarInfo& varInfo, llvm::DIType type, llvm::Value* varValue);
		
		Optional<llvm::DebugLoc> getDebugLocation(Function& function, const SEM::Value& value);
		
	}
	
}

#endif
