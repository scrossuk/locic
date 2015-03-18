#ifndef LOCIC_CODEGEN_TARGETOPTIONS_HPP
#define LOCIC_CODEGEN_TARGETOPTIONS_HPP

#include <string>

namespace locic {
	
	namespace CodeGen {
		
		struct TargetOptions {
			std::string triple;
			std::string arch;
			std::string cpu;
			std::string floatABI;
			std::string fpu;
		};
		
	}
	
}

#endif
