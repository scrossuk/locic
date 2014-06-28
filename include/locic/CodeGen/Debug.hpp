#ifndef LOCIC_CODEGEN_DEBUG_HPP
#define LOCIC_CODEGEN_DEBUG_HPP

#include <string>
#include <vector>

#include <boost/optional.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/Debug.hpp>
#include <locic/Name.hpp>
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
		
		class DebugBuilder {
			public:
				DebugBuilder(Module& module);
				~DebugBuilder();
				
				void finalize();
				
				llvm::DICompileUnit createCompileUnit(const DebugCompileUnit& compileUnit);
				
				llvm::DICompileUnit compileUnit() const;
				
				llvm::DIFile createFile(const std::string& path);
				
				llvm::DISubprogram createFunction(llvm::DIFile file, unsigned int lineNumber, bool isDefinition, const Name& name, llvm::DIType functionType, llvm::Function* function);
				
				llvm::DIVariable createVar(llvm::DIDescriptor scope, bool isParam, const std::string& name, llvm::DIFile file, unsigned lineNumber, llvm::DIType type);
				
				llvm::DIType createVoidType();
				
				llvm::DIType createNullType();
				
				llvm::DIType createReferenceType(llvm::DIType type);
				
				llvm::DIType createPointerType(llvm::DIType type);
				
				llvm::DIType createIntType(const std::string& name);
				
				llvm::DIType createObjectType(llvm::DIFile file, unsigned int lineNumber, const Name& name);
				
				llvm::DIType createFunctionType(llvm::DIFile file, const std::vector<llvm::Value*>& parameters);
				
				llvm::Instruction* insertVariableDeclare(Function& function, llvm::DIVariable variable, llvm::Value* varValue);
				
			private:
				Module& module_;
				llvm::DIBuilder builder_;
			
		};
		
		std::pair<std::string, std::string> splitPath(const std::string& path);
		
		llvm::DISubprogram genDebugFunction(Module& module, const Debug::FunctionInfo& functionInfo, llvm::DIType functionType, llvm::Function* function);
		
		llvm::Instruction* genDebugVar(Function& function, const Debug::VarInfo& varInfo, llvm::DIType type, llvm::Value* varValue);
		
		boost::optional<llvm::DebugLoc> getDebugLocation(Function& function, SEM::Value* value);
		
	}
	
}

#endif
