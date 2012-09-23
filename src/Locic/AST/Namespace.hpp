#ifndef LOCIC_AST_NAMESPACE_HPP
#define LOCIC_AST_NAMESPACE_HPP

#include <string>
#include <vector>

#include <Locic/AST/Function.hpp>
#include <Locic/AST/TypeInstance.hpp>

namespace AST {

	struct Namespace {
		std::string name;
		std::vector<Function *> functions;
		std::vector<Namespace *> namespaces;
		std::vector<TypeInstance *> typeInstances;
		
		inline Namespace(const std::string& n)
			: name(n) { }
			
		inline void print() {
			// TODO
		}
	};
	
}

#endif
