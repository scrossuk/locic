#ifndef LOCIC_AST_MODULE_HPP
#define LOCIC_AST_MODULE_HPP

#include <string>
#include <vector>

#include <Locic/AST/Function.hpp>
#include <Locic/AST/TypeInstance.hpp>

namespace AST {

	struct Module {
		std::string name;
		std::vector<Function *> functions;
		std::vector<TypeInstance *> typeInstances;
		
		inline Module(const std::string& n)
			: name(n) { }
			
		inline void print() {
			// TODO
		}
	};
	
}

#endif
