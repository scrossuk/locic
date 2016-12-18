#ifndef LOCIC_AST_NAMESPACEDECL_HPP
#define LOCIC_AST_NAMESPACEDECL_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class Alias;
		class Function;
		class ModuleScopeDecl;
		class Namespace;
		class NamespaceDecl;
		class StaticAssert;
		struct TypeInstance;
		
		struct NamespaceData {
			std::vector<Node<Alias>> aliases;
			std::vector<Node<Function>> functions;
			std::vector<Node<ModuleScopeDecl>> moduleScopes;
			std::vector<Node<NamespaceDecl>> namespaces;
			std::vector<Node<StaticAssert>> staticAsserts;
			std::vector<Node<TypeInstance>> typeInstances;
			
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
			NamespaceDecl(const String& n, Node<NamespaceData> d);
			~NamespaceDecl();
			
			String name() const;
			
			const Node<NamespaceData>& data() const;
			
			void setNamespace(Namespace& nameSpace);
			
			Namespace& nameSpace();
			const Namespace& nameSpace() const;
			
			std::string toString() const;
			
		private:
			String name_;
			Node<NamespaceData> data_;
			Namespace* namespace_;
			
		};
		
		typedef std::vector<Node<NamespaceDecl>> NamespaceList;
		
	}
	
}

#endif
