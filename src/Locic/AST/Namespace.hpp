#ifndef LOCIC_AST_NAMESPACE_HPP
#define LOCIC_AST_NAMESPACE_HPP

#include <string>
#include <vector>

#include <Locic/AST/Function.hpp>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/TypeInstance.hpp>

namespace AST {

	struct Namespace;
	
	struct NamespaceData {
		std::vector< AST::Node<Function> > functions;
		std::vector< AST::Node<Namespace> > namespaces;
		std::vector< AST::Node<TypeInstance> > typeInstances;
	};
	
	struct Namespace {
		std::string name;
		AST::Node<NamespaceData> data;
		
		inline Namespace(const std::string& n, AST::Node<NamespaceData> d)
			: name(n), data(d) { }
	};
	
	typedef std::vector<Node<Namespace>> NamespaceList;
	
}

#endif
