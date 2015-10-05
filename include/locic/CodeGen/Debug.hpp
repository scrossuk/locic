#ifndef LOCIC_CODEGEN_DEBUG_HPP
#define LOCIC_CODEGEN_DEBUG_HPP

#include <string>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/Debug.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	class PrimitiveID;
	
	namespace SEM {
		
		class Function;
		class Value;
		
	}
	
	namespace CodeGen {
		
		struct DebugCompileUnit {
			std::string compilerName;
			std::string directoryName;
			std::string fileName;
			std::string flags;
		};
		
		class Function;
		class Module;
		
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
				
				llvm::DIType createUnspecifiedType(const String& name);
				
				llvm::DIType createVoidType();
				
				llvm::DIType createNullType();
				
				llvm::DIType createReferenceType(llvm::DIType type);
				
				llvm::DIType createPointerType(llvm::DIType type);
				
				llvm::DIType createIntType(PrimitiveID primitiveID);
				
				llvm::DIType createObjectType(llvm::DIFile file, unsigned int lineNumber, const Name& name);
				
				llvm::DIType createFunctionType(llvm::DIFile file, const std::vector<LLVMMetadataValue*>& parameters);
				
				llvm::Instruction* insertVariableDeclare(Function& function, llvm::DIVariable variable, llvm::Value* varValue);
				
			private:
				Module& module_;
				llvm::DIBuilder builder_;
			
		};
		
		std::pair<std::string, std::string> splitPath(const std::string& path);
		
		llvm::DISubprogram genDebugFunction(Module& module, const Debug::FunctionInfo& functionInfo, llvm::DIType functionType,
			llvm::Function* function, bool isInternal, bool isDefinition);
		
		Optional<llvm::DISubprogram> genDebugFunctionInfo(Module& module, const SEM::Function* function, llvm::Function* llvmFunction);
		
		llvm::Instruction* genDebugVar(Function& function, const Debug::VarInfo& varInfo, llvm::DIType type, llvm::Value* varValue);
		
		llvm::DebugLoc getDebugLocation(Function& function, const Debug::SourceLocation& debugSourceLocation);
		
		Optional<llvm::DebugLoc> getFunctionDebugLocation(Function& function, const SEM::Function& semFunction);
		
		Optional<llvm::DebugLoc> getValueDebugLocation(Function& function, const SEM::Value& value);
		
	}
	
}

#endif
