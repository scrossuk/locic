#ifndef LOCIC_AST_VAR_HPP
#define LOCIC_AST_VAR_HPP

#include <string>
#include <vector>

#include <locic/Debug/VarInfo.hpp>
#include <locic/Support/Optional.hpp>
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
			
			static Var* Any(Node<TypeDecl> type);
			
			static Var* NamedVar(Node<TypeDecl> type, String name);
			
			static Var* PatternVar(Node<TypeDecl> type,
			                       Node<VarList> varList);
			
			Kind kind() const;
			
			Node<TypeDecl>& type();
			const Node<TypeDecl>& type() const;
			
			bool isAny() const;
			
			bool isNamed() const;
			const String& name() const;
			
			bool isPattern() const;
			Node<VarList>& varList();
			const Node<VarList>& varList() const;
			
			bool isFinal() const;
			void setFinal();
			
			bool isOverrideConst() const;
			void setOverrideConst();
			
			bool isUnused() const;
			void setUnused();
			
			size_t index() const;
			void setIndex(size_t index);
			
			void setDebugInfo(Debug::VarInfo debugInfo);
			Optional<Debug::VarInfo> debugInfo() const;
			
			std::string toString() const;
			
		private:
			Var(Kind pKind, Node<TypeDecl> type);
			
			Kind kind_;
			bool isFinal_;
			bool isOverrideConst_;
			bool isUnused_;
			size_t index_;
			
			Node<TypeDecl> type_;
			
			struct {
				String name;
			} namedVar_;
			
			struct {
				Node<VarList> varList;
			} patternVar_;
			
			Optional<Debug::VarInfo> debugInfo_;
			
		};
		
	}
	
}

#endif 
