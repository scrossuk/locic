#ifndef LOCIC_CODEGEN_DEBUG_HPP
#define LOCIC_CODEGEN_DEBUG_HPP

#include <string>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/SourceLocation.hpp>

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
		
		class DebugBuilder {
			public:
				DebugBuilder(Module& module);
				~DebugBuilder();
				
				void finalize();
				
				llvm::DICompileUnit createCompileUnit(const DebugCompileUnit& compileUnit);
				
				llvm::DICompileUnit compileUnit() const;
				
				llvm::DIFile createFile(const std::string& fileName, const std::string& directory);
				
				llvm::DISubprogram createFunction(llvm::DIFile file, unsigned int lineNumber, bool isDefinition, const Name& name, llvm::DIType functionType, llvm::Function* function);
				
				llvm::DIVariable createVar(llvm::DIDescriptor scope, bool isParam, const std::string& name, llvm::DIFile file, unsigned lineNumber, llvm::DIType type);
				
				llvm::DIType createVoidType();
				
				llvm::DIType createNullType();
				
				llvm::DIType createReferenceType(llvm::DIType type);
				
				llvm::DIType createPointerType(llvm::DIType type);
				
				llvm::DIType createObjectType(llvm::DIFile file, unsigned int lineNumber, const Name& name);
				
				llvm::DIType createFunctionType(llvm::DIFile file, const std::vector<llvm::Value*>& parameters);
				
				llvm::Instruction* insertVariableDeclare(Function& function, llvm::DIVariable variable, llvm::Value* varValue);
				
			private:
				Module& module_;
				llvm::DIBuilder builder_;
			
		};
		
		llvm::Instruction* genDebugVar(Function& function, const SourceLocation& sourceLocation, bool isParam, const std::string& name, llvm::DIType type, llvm::Value* varValue);
		
	}
	
}

#endif
