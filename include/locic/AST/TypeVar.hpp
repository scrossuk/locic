#ifndef LOCIC_AST_TYPEVAR_HPP
#define LOCIC_AST_TYPEVAR_HPP

#include <string>
#include <vector>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		class TypeVar;
		
		typedef std::vector<Node<TypeVar>> TypeVarList;
		
		class TypeVar {
		public:
			enum Kind {
				NAMEDVAR,
				PATTERNVAR,
				ANYVAR
			};
			
			static TypeVar* NamedVar(const Node<Type>& type, String name);
			
			static TypeVar* PatternVar(const Node<Type>& type, const Node<TypeVarList>& typeVarList);
			
			static TypeVar* Any();
			
			Kind kind() const;
			
			bool isNamed() const;
			const Node<Type>& namedType() const;
			const String& name() const;
			
			bool isFinal() const;
			void setFinal();
			
			bool isOverrideConst() const;
			void setOverrideConst();
			
			bool isUnused() const;
			void setUnused();
			
			bool isPattern() const;
			const Node<Type>& patternType() const;
			const Node<TypeVarList>& typeVarList() const;
			
			bool isAny() const;
			
			std::string toString() const;
			
		private:
			TypeVar(Kind pKind);
			
			Kind kind_;
			
			struct {
				bool isFinal;
				bool isOverrideConst;
				bool isUnused;
				Node<Type> type;
				String name;
			} namedVar_;
			
			struct {
				Node<Type> type;
				Node<TypeVarList> typeVarList;
			} patternVar_;
			
		};
		
	}
	
}

#endif 
