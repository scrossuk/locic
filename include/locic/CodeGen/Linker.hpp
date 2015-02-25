#ifndef LOCIC_CODEGEN_LINKER_HPP
#define LOCIC_CODEGEN_LINKER_HPP

#include <memory>
#include <string>

namespace locic {
	
	namespace CodeGen {
		
		class Context;
		class Module;
		
		class Linker {
			public:
				Linker(Context& context, Module& module);
				~Linker();
				
				void loadModule(const std::string& fileName);
				
				Module& module();
				
				const Module& module() const;
				
			private:
				std::unique_ptr<class LinkerImpl> impl_;
				
		};
		
	}
	
}

#endif
