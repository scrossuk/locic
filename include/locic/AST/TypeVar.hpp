#ifndef LOCIC_AST_TYPEVAR_HPP
#define LOCIC_AST_TYPEVAR_HPP

#include <string>
#include <vector>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		struct TypeVar;
		
		typedef std::vector<Node<TypeVar>> TypeVarList;
		
		struct TypeVar {
			enum Kind {
				NAMEDVAR,
				PATTERNVAR,
				ANYVAR
			} kind;
			
			struct {
				bool isFinal;
				bool isUnused;
				Node<Type> type;
				String name;
			} namedVar;
			
			struct {
				Node<Type> type;
				Node<TypeVarList> typeVarList;
			} patternVar;
			
			public:
				static TypeVar* NamedVar(const Node<Type>& type, String name);
				
				static TypeVar* FinalNamedVar(const Node<Type>& type, String name);
				
				static TypeVar* UnusedNamedVar(const Node<Type>& type, String name);
				
				static TypeVar* UnusedFinalNamedVar(const Node<Type>& type, String name);
				
				static TypeVar* PatternVar(const Node<Type>& type, const Node<TypeVarList>& typeVarList);
				
				static TypeVar* Any();
				
				std::string toString() const;
			
			private:
				TypeVar(Kind pKind);
			
		};
		
	}
	
}

#endif 
