#ifndef LOCIC_SEM_NAMESPACE_HPP
#define LOCIC_SEM_NAMESPACE_HPP

#include <map>
#include <string>

#include <locic/Name.hpp>

namespace locic {

	namespace SEM {
		
		class Function;
		class Namespace;
		class TypeAlias;
		class TypeInstance;
		
		class NamespaceItem {
			public:
				enum Kind {
					FUNCTION,
					NAMESPACE,
					TYPEALIAS,
					TYPEINSTANCE
				};
				
				static NamespaceItem Function(Function* function);
				
				static NamespaceItem Namespace(Namespace* nameSpace);
				
				static NamespaceItem TypeAlias(TypeAlias* typeAlias);
				
				static NamespaceItem TypeInstance(TypeInstance* typeInstance);
				
				Kind kind() const;
				
				bool isFunction() const;
				bool isNamespace() const;
				bool isTypeAlias() const;
				bool isTypeInstance() const;
				
				SEM::Function* function() const;
				SEM::Namespace* nameSpace() const;
				SEM::TypeAlias* typeAlias() const;
				SEM::TypeInstance* typeInstance() const;
				
				std::string toString() const;
				
			private:
				NamespaceItem(Kind pKind);
				
				Kind kind_;
				
				union {
					void* ptr;
					SEM::Function* function;
					SEM::Namespace* nameSpace;
					SEM::TypeAlias* typeAlias;
					SEM::TypeInstance* typeInstance;
				} data_;
				
		};
		
		class Namespace {
			public:
				Namespace(const Name& n);
					
				const Name& name() const;
				
				std::map<std::string, NamespaceItem>& items();
				const std::map<std::string, NamespaceItem>& items() const;
				
				std::string toString() const;
				
			private:
				Name name_;
				std::map<std::string, NamespaceItem> items_;
				
		};
		
	}
	
}

#endif
