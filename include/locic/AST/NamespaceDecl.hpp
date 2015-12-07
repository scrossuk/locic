#ifndef LOCIC_AST_NAMESPACEDECL_HPP
#define LOCIC_AST_NAMESPACEDECL_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
		
		class Namespace;
		
	}
	
	namespace AST {
		
		class AliasDecl;
		struct Function;
		struct ModuleScope;
		class NamespaceDecl;
		class StaticAssert;
		struct TypeInstance;
		
		struct NamespaceData {
			std::vector< AST::Node<AliasDecl> > aliases;
			std::vector< AST::Node<Function> > functions;
			std::vector< AST::Node<ModuleScope> > moduleScopes;
			std::vector< AST::Node<NamespaceDecl> > namespaces;
			std::vector< AST::Node<StaticAssert> > staticAsserts;
			std::vector< AST::Node<TypeInstance> > typeInstances;
			
			std::string toString() const;
		};
		
		class NamespaceDecl {
		public:
			NamespaceDecl(const String& n, AST::Node<NamespaceData> d);
				
			String name() const;
			
			const AST::Node<NamespaceData>& data() const;
			
			void setNamespace(SEM::Namespace& nameSpace);
			
			SEM::Namespace& nameSpace();
			const SEM::Namespace& nameSpace() const;
			
			std::string toString() const;
			
		private:
			String name_;
			AST::Node<NamespaceData> data_;
			SEM::Namespace* namespace_;
			
		};
		
		typedef std::vector<Node<NamespaceDecl>> NamespaceList;
		
	}
	
}

#endif
