#ifndef LOCIC_AST_NAMESPACE_HPP
#define LOCIC_AST_NAMESPACE_HPP

#include <map>
#include <memory>
#include <string>

#include <locic/AST/GlobalStructure.hpp>
#include <locic/AST/NamespaceDecl.hpp>
#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace AST {
		
		class Alias;
		class Function;
		class Namespace;
		
		class NamespaceItem {
			public:
				enum Kind {
					ALIAS,
					FUNCTION,
					NAMESPACE,
					TYPEINSTANCE
				};
				
				static NamespaceItem Alias(AST::Alias& alias);
				
				static NamespaceItem Function(AST::Function& function);
				
				static NamespaceItem Namespace(std::unique_ptr<AST::Namespace> nameSpace);
				
				static NamespaceItem TypeInstance(std::unique_ptr<SEM::TypeInstance> typeInstance);
				
				NamespaceItem(NamespaceItem&&) = default;
				NamespaceItem& operator=(NamespaceItem&) = default;
				
				~NamespaceItem();
				
				Kind kind() const;
				
				bool isAlias() const;
				bool isFunction() const;
				bool isNamespace() const;
				bool isTypeInstance() const;
				
				AST::Alias& alias() const;
				AST::Function& function() const;
				AST::Namespace& nameSpace() const;
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
					AST::Alias* alias;
					AST::Function* function;
					AST::Namespace* nameSpace;
					SEM::TypeInstance* typeInstance;
				} data_;
				
		};
		
		/**
		 * \brief Uniqued namespace
		 * 
		 * This class is created when multiple namespace declarations
		 * are 'uniqued. For example:
		 * 
		 * namespace Test { void f(); }
		 * namespace Test { void g(); }
		 * 
		 * In this case there are two namespace declarations (both
		 * called 'Test') but just one uniqued namespace ('Test').
		 */
		class Namespace {
			public:
				// Create root namespace.
				Namespace();
				
				Namespace(Name name, GlobalStructure parent);
				
				std::vector<Node<NamespaceDecl>*>&
				namespaceDecls();
				const std::vector<Node<NamespaceDecl>*>&
				namespaceDecls() const;
				
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
				std::vector<Node<NamespaceDecl>*> namespaceDecls_;
				
		};
		
	}
	
}

#endif
