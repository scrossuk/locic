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
		class FunctionDecl;
		class ModuleScopeDecl;
		class NamespaceDecl;
		class StaticAssert;
		struct TypeInstance;
		
		struct NamespaceData {
			std::vector< AST::Node<AliasDecl> > aliases;
			std::vector< AST::Node<FunctionDecl> > functions;
			std::vector< AST::Node<ModuleScopeDecl> > moduleScopes;
			std::vector< AST::Node<NamespaceDecl> > namespaces;
			std::vector< AST::Node<StaticAssert> > staticAsserts;
			std::vector< AST::Node<TypeInstance> > typeInstances;
			
			NamespaceData();
			~NamespaceData();
			
			NamespaceData(const NamespaceData&) = delete;
			NamespaceData& operator=(const NamespaceData&) = delete;
			
			NamespaceData(NamespaceData&&) = default;
			NamespaceData& operator=(NamespaceData&&) = delete;
			
			bool empty() const {
				return aliases.empty() && functions.empty() &&
				       moduleScopes.empty() && namespaces.empty() &&
				       staticAsserts.empty() && typeInstances.empty();
			}
			
			std::string toString() const;
		};
		
		class NamespaceDecl {
		public:
			NamespaceDecl(const String& n, AST::Node<NamespaceData> d);
			~NamespaceDecl();
			
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
