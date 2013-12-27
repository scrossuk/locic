#ifndef LOCIC_AST_TYPEVAR_HPP
#define LOCIC_AST_TYPEVAR_HPP

#include <string>
#include <vector>

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
				Node<Type> type;
				std::string name;
			} namedVar;
			
			struct {
				Node<Type> type;
				Node<TypeVarList> typeVarList;
			} patternVar;
			
			public:	
				static TypeVar* NamedVar(const Node<Type>& type, const std::string& name);
				
				static TypeVar* PatternVar(const Node<Type>& type, const Node<TypeVarList>& typeVarList);
				
				static TypeVar* Any();
				
				std::string toString() const;
			
			private:
				TypeVar(Kind pKind);
			
		};
		
	}
	
}

#endif 
