#ifndef LOCIC_SEM_MODULE_HPP
#define LOCIC_SEM_MODULE_HPP

#include <list>
#include <map>
#include <string>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM{

	struct Module{
		std::string name;
		std::map<std::string, TypeInstance *> typeInstances;
		std::map<std::string, Function *> functions;
		
		inline Module(const std::string& n)
			: name(n){ }
	};
	
}

#endif
