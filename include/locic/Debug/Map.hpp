#ifndef LOCIC_DEBUG_MAP_HPP
#define LOCIC_DEBUG_MAP_HPP

#include <map>
#include <string>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/VarInfo.hpp>

#include <locic/SEM.hpp>

namespace locic {

	namespace Debug {
		
		struct Map {
			std::map<SEM::Var*, VarInfo> varMap;
		};
		
	}
	
}

#endif
