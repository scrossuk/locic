#ifndef LOCIC_SEM_GLOBALSTRUCTURE_HPP
#define LOCIC_SEM_GLOBALSTRUCTURE_HPP

namespace locic {
	
	namespace SEM {
		
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
				
				static GlobalStructure Alias(Alias& alias);
				
				static GlobalStructure Namespace(Namespace& nameSpace);
				
				static GlobalStructure TypeInstance(TypeInstance& typeInstance);
				
				GlobalStructure(GlobalStructure&&) = default;
				GlobalStructure& operator=(GlobalStructure&&) = default;
				
				Kind kind() const;
				
				bool isAlias() const;
				bool isNamespace() const;
				bool isTypeInstance() const;
				
				GlobalStructure& parent();
				const GlobalStructure& parent() const;
				
				SEM::Alias& alias();
				const SEM::Alias& alias() const;
				
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
					SEM::Alias* alias;
					SEM::Namespace* nameSpace;
					SEM::TypeInstance* typeInstance;
				} data_;
				
		};
		
	}
	
}

#endif
