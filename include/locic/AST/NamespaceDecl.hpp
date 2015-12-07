#ifndef LOCIC_AST_NAMESPACEDECL_HPP
#define LOCIC_AST_NAMESPACEDECL_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		struct Alias;
		struct Function;
		struct ModuleScope;
		struct NamespaceDecl;
		class StaticAssert;
		struct TypeInstance;
		
		struct NamespaceData {
			std::vector< AST::Node<Alias> > aliases;
			std::vector< AST::Node<Function> > functions;
			std::vector< AST::Node<ModuleScope> > moduleScopes;
			std::vector< AST::Node<NamespaceDecl> > namespaces;
			std::vector< AST::Node<StaticAssert> > staticAsserts;
			std::vector< AST::Node<TypeInstance> > typeInstances;
			
			std::string toString() const;
		};
		
		struct NamespaceDecl {
			String name;
			AST::Node<NamespaceData> data;
			
			NamespaceDecl(const String& n, AST::Node<NamespaceData> d);
				
			std::string toString() const;
		};
		
		typedef std::vector<Node<NamespaceDecl>> NamespaceList;
		
	}
	
}

#endif
