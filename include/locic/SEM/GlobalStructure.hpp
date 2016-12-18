#ifndef LOCIC_SEM_GLOBALSTRUCTURE_HPP
#define LOCIC_SEM_GLOBALSTRUCTURE_HPP

namespace locic {
	
	class Name;
	
	namespace AST {
		
		class AliasDecl;
		
	}
	
	namespace SEM {
		
		class Namespace;
		class TypeInstance;
		
		class GlobalStructure {
			public:
				enum Kind {
					ALIAS,
					NAMESPACE,
					TYPEINSTANCE
				};
				
				static GlobalStructure Alias(AST::AliasDecl& alias);
				
				static GlobalStructure Namespace(Namespace& nameSpace);
				
				static GlobalStructure TypeInstance(TypeInstance& typeInstance);
				
				GlobalStructure(GlobalStructure&&) = default;
				GlobalStructure& operator=(GlobalStructure&&) = default;
				
				Kind kind() const;
				
				bool isAlias() const;
				bool isNamespace() const;
				bool isTypeInstance() const;
				
				const Name& name() const;
				
				GlobalStructure& parent();
				const GlobalStructure& parent() const;
				
				AST::AliasDecl& alias();
				const AST::AliasDecl& alias() const;
				
				SEM::Namespace& nameSpace();
				const SEM::Namespace& nameSpace() const;
				
				SEM::TypeInstance& typeInstance();
				const SEM::TypeInstance& typeInstance() const;
				
				SEM::Namespace& nextNamespace();
				const SEM::Namespace& nextNamespace() const;
				
				std::string toString() const;
				
			private:
				GlobalStructure(Kind pKind);
				
				GlobalStructure(const GlobalStructure&) = delete;
				GlobalStructure& operator=(const GlobalStructure&) = delete;
				
				Kind kind_;
				
				union {
					void* ptr;
					AST::AliasDecl* alias;
					SEM::Namespace* nameSpace;
					SEM::TypeInstance* typeInstance;
				} data_;
				
		};
		
	}
	
}

#endif
