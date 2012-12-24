#ifndef LOCIC_CODEGEN_HPP
#define LOCIC_CODEGEN_HPP

#include <cstddef>
#include <string>
#include <Locic/SEM.hpp>

class CodeGen;

namespace Locic{

	class CodeGenerator{
		public:
			CodeGenerator(const std::string& moduleName);
			~CodeGenerator();
			
			void applyOptimisations(size_t optLevel);
			
			void genNamespace(SEM::Namespace * nameSpace);
			
			void writeToFile(const std::string& fileName);
			
			void dumpToFile(const std::string& fileName);
			
			void dump();
			
		private:
			CodeGen * codeGen_;
		
	};	
	
}

#endif
