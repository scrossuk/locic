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
			
			Node<TypeDecl>& declType();
			const Node<TypeDecl>& declType() const;
			
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
			
			bool isMarkedUnused() const;
			void setMarkedUnused();
			
			bool isUsed() const;
			void setUsed();
			
			/**
			 * \brief Get construct type.
			 * 
			 * This is the type of the value being held in this
			 * variable. When this variable is initialised it must
			 * be passed a value of this type.
			 * 
			 * If the variable itself has an 'lval' type, then the
			 * lval type and construct type are the same.
			 */
			const SEM::Type* constructType() const;
			void setConstructType(const SEM::Type* type);
			
			/**
			 * \brief Get lval type.
			 * 
			 * This is the type of the lvalue holding this variable.
			 * 
			 * If the variable itself has an 'lval' type, then the
			 * lval type and construct type are the same.
			 */
			const SEM::Type* lvalType() const;
			void setLvalType(const SEM::Type* type);
			
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
			bool isMarkedUnused_;
			bool isUsed_;
			size_t index_;
			
			Node<TypeDecl> type_;
			
			struct {
				String name;
			} namedVar_;
			
			struct {
				Node<VarList> varList;
			} patternVar_;
			
			Optional<Debug::VarInfo> debugInfo_;
			const SEM::Type* constructType_;
			const SEM::Type* lvalType_;
			
		};
		
	}
	
}

#endif 
