#ifndef LOCIC_AST_GLOBALSTRUCTURE_HPP
#define LOCIC_AST_GLOBALSTRUCTURE_HPP

namespace locic {
	
	class Name;
	
	namespace AST {
		
		class Alias;
		class Namespace;
		class TypeInstance;
		
		class GlobalStructure {
			public:
				enum Kind {
					ALIAS,
					NAMESPACE,
					TYPEINSTANCE
				};
				
				static GlobalStructure Alias(AST::Alias& alias);
				
				static GlobalStructure Namespace(AST::Namespace& nameSpace);
				
				static GlobalStructure TypeInstance(AST::TypeInstance& typeInstance);
				
				GlobalStructure(GlobalStructure&&) = default;
				GlobalStructure& operator=(GlobalStructure&&) = default;
				
				Kind kind() const;
				
				bool isAlias() const;
				bool isNamespace() const;
				bool isTypeInstance() const;
				
				const Name& name() const;
				
				GlobalStructure& parent();
				const GlobalStructure& parent() const;
				
				AST::Alias& alias();
				const AST::Alias& alias() const;
				
				AST::Namespace& nameSpace();
				const AST::Namespace& nameSpace() const;
				
				AST::TypeInstance& typeInstance();
				const AST::TypeInstance& typeInstance() const;
				
				AST::Namespace& nextNamespace();
				const AST::Namespace& nextNamespace() const;
				
				std::string toString() const;
				
			private:
				GlobalStructure(Kind pKind);
				
				GlobalStructure(const GlobalStructure&) = delete;
				GlobalStructure& operator=(const GlobalStructure&) = delete;
				
				Kind kind_;
				
				union {
					void* ptr;
					AST::Alias* alias;
					AST::Namespace* nameSpace;
					AST::TypeInstance* typeInstance;
				} data_;
				
		};
		
	}
	
}

#endif
