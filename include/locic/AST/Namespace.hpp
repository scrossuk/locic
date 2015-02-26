#ifndef LOCIC_AST_NAMESPACE_HPP
#define LOCIC_AST_NAMESPACE_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/String.hpp>

namespace locic {

	namespace AST {
	
		struct Function;
		struct ModuleScope;
		struct Namespace;
		struct TypeAlias;
		struct TypeInstance;
		
		struct NamespaceData {
			std::vector< AST::Node<Function> > functions;
			std::vector< AST::Node<ModuleScope> > moduleScopes;
			std::vector< AST::Node<Namespace> > namespaces;
			std::vector< AST::Node<TypeAlias> > typeAliases;
			std::vector< AST::Node<TypeInstance> > typeInstances;
			
			std::string toString() const;
		};
		
		struct Namespace {
			String name;
			AST::Node<NamespaceData> data;
			
			Namespace(const String& n, AST::Node<NamespaceData> d);
				
			std::string toString() const;
		};
		
		typedef std::vector<Node<Namespace>> NamespaceList;
		
	}
	
}

#endif
