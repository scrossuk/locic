#ifndef LOCIC_AST_MODULE_HPP
#define LOCIC_AST_MODULE_HPP

#include <string>
#include <vector>

#include <Locic/AST/Namespace.hpp>

namespace AST {

	struct Module {
		std::string name;
		AST::Namespace * nameSpace;
		
		inline Module(const std::string& n, AST::Namespace * ns)
			: name(n), nameSpace(ns) { }
			
		inline void print() {
			// TODO
		}
	};
	
}

#endif
