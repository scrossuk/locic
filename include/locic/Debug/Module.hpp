#ifndef LOCIC_DEBUG_MODULE_HPP
#define LOCIC_DEBUG_MODULE_HPP

#include <map>

#include <locic/Debug/CompilerInfo.hpp>
#include <locic/Debug/FunctionInfo.hpp>
#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/StatementInfo.hpp>
#include <locic/Debug/TypeInstanceInfo.hpp>
#include <locic/Debug/ValueInfo.hpp>
#include <locic/Debug/VarInfo.hpp>

#include <locic/SEM.hpp>

namespace locic {

	namespace Debug {
		
		class Module {
		public:
			CompilerInfo compiler;
			std::map<SEM::Function*, FunctionInfo> functionMap;
			std::map<SEM::Statement*, StatementInfo> statementMap;
			std::map<SEM::TypeInstance*, TypeInstanceInfo> typeInstanceMap;
		};
		
	}
	
}

#endif
