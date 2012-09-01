#ifndef LOCIC_SEM_MODULE_HPP
#define LOCIC_SEM_MODULE_HPP

#include <string>
#include <vector>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM{

	struct Module{
		std::string name;
		std::vector<TypeInstance *> typeInstances;
		std::vector<Function *> functions;
		
		inline Module(const std::string& n)
			: name(n){ }
	};
	
}

#endif
