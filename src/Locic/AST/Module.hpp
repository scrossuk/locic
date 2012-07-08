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
		
		inline void addTypeInstance(TypeInstance * typeInstance){
			typeInstances.push_back(typeInstance);
			
			for(std::size_t i = 0; i < typeInstance->functions.size(); i++){
				functions.push_back(typeInstance->functions.at(i));
			}
		}
			
		inline void print() {
			// TODO
		}
	};
	
}

#endif
