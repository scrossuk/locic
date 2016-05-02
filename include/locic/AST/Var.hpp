#ifndef LOCIC_AST_VAR_HPP
#define LOCIC_AST_VAR_HPP

#include <string>
#include <vector>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TypeDecl.hpp>

namespace locic {

	namespace AST {
	
		class Var;
		
		typedef std::vector<Node<Var>> VarList;
		
		class Var {
		public:
			enum Kind {
				NAMEDVAR,
				PATTERNVAR,
				ANYVAR
			};
			
			static Var* NamedVar(Node<TypeDecl> type, String name);
			
			static Var* PatternVar(Node<TypeDecl> type,
			                       Node<VarList> varList);
			
			static Var* Any();
			
			Kind kind() const;
			
			bool isNamed() const;
			const Node<TypeDecl>& namedType() const;
			const String& name() const;
			
			bool isFinal() const;
			void setFinal();
			
			bool isOverrideConst() const;
			void setOverrideConst();
			
			bool isUnused() const;
			void setUnused();
			
			bool isPattern() const;
			const Node<TypeDecl>& patternType() const;
			const Node<VarList>& varList() const;
			
			bool isAny() const;
			
			std::string toString() const;
			
		private:
			Var(Kind pKind);
			
			Kind kind_;
			
			struct {
				bool isFinal;
				bool isOverrideConst;
				bool isUnused;
				Node<TypeDecl> type;
				String name;
			} namedVar_;
			
			struct {
				Node<TypeDecl> type;
				Node<VarList> varList;
			} patternVar_;
			
		};
		
	}
	
}

#endif 
