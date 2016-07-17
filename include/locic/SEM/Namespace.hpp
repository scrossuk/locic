#ifndef LOCIC_SEM_NAMESPACE_HPP
#define LOCIC_SEM_NAMESPACE_HPP

#include <map>
#include <memory>
#include <string>

#include <locic/AST/NamespaceDecl.hpp>
#include <locic/SEM/GlobalStructure.hpp>
#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
		
		class Alias;
		class Function;
		class Namespace;
		class TypeInstance;
		
		class NamespaceItem {
			public:
				enum Kind {
					ALIAS,
					FUNCTION,
					NAMESPACE,
					TYPEINSTANCE
				};
				
				static NamespaceItem Alias(std::unique_ptr<Alias> alias);
				
				static NamespaceItem Function(std::unique_ptr<Function> function);
				
				static NamespaceItem Namespace(std::unique_ptr<Namespace> nameSpace);
				
				static NamespaceItem TypeInstance(std::unique_ptr<TypeInstance> typeInstance);
				
				NamespaceItem(NamespaceItem&&) = default;
				NamespaceItem& operator=(NamespaceItem&) = default;
				
				~NamespaceItem();
				
				Kind kind() const;
				
				bool isAlias() const;
				bool isFunction() const;
				bool isNamespace() const;
				bool isTypeInstance() const;
				
				SEM::Alias& alias() const;
				SEM::Function& function() const;
				SEM::Namespace& nameSpace() const;
				SEM::TypeInstance& typeInstance() const;
				
				Debug::SourceLocation location() const;
				
				std::string toString() const;
				
			private:
				NamespaceItem(Kind pKind);
				
				NamespaceItem(const NamespaceItem&) = delete;
				NamespaceItem& operator=(const NamespaceItem&) = delete;
				
				Kind kind_;
				
				union {
					void* ptr;
					SEM::Alias* alias;
					SEM::Function* function;
					SEM::Namespace* nameSpace;
					SEM::TypeInstance* typeInstance;
				} data_;
				
		};
		
		class Namespace {
			public:
				// Create root namespace.
				Namespace();
				
				Namespace(Name name, GlobalStructure parent);
				
				std::vector<AST::Node<AST::NamespaceDecl>*>&
				astNamespaces();
				const std::vector<AST::Node<AST::NamespaceDecl>*>&
				astNamespaces() const;
				
				GlobalStructure& parent();
				const GlobalStructure& parent() const;
				
				const Name& name() const;
				
				FastMap<String, NamespaceItem>& items();
				const FastMap<String, NamespaceItem>& items() const;
				
				std::string toString() const;
				
			private:
				GlobalStructure parent_;
				Name name_;
				FastMap<String, NamespaceItem> items_;
				std::vector<AST::Node<AST::NamespaceDecl>*> astNamespaces_;
				
		};
		
	}
	
}

#endif
