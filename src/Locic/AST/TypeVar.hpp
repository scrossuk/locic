#ifndef LOCIC_AST_TYPEVAR_HPP
#define LOCIC_AST_TYPEVAR_HPP

#include <string>
#include <Locic/Optional.hpp>
#include <Locic/String.hpp>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/Type.hpp>

namespace AST {

	struct TypeVar {
		Node<Type> type;
		std::string name;
		bool usesCustomLval;
		
		inline TypeVar(Node<Type> t, const std::string& n, bool u)
			: type(t), name(n), usesCustomLval(u) { }
		
		inline std::string toString() const {
			return Locic::makeString("TypeVar(isLval = %s, type = %s, name = %s)",
				usesCustomLval ? "YES" : "NO",
				type.toString().c_str(), name.c_str());
		}
	};
	
	typedef std::vector<Node<TypeVar>> TypeVarList;
	
	struct PatternVar {
		Locic::Optional<Node<TypeVar>> typeVar;
		
		static inline PatternVar* MatchAny() {
			return new PatternVar(Locic::Optional<Node<TypeVar>>());
		}
		
		static inline PatternVar* MatchTypeVar(const Node<TypeVar>& typeVar) {
			return new PatternVar(Locic::make_optional(typeVar));
		}
		
		inline PatternVar(const Locic::Optional<Node<TypeVar>>& pTypeVar)
			: typeVar(pTypeVar) { }
		
		inline std::string toString() const {
			if (typeVar.hasValue()) {
				return Locic::makeString("PatternVar[MatchTypeVar](typeVar = %s)",
					typeVar.getValue()->toString().c_str());
			} else {
				return "PatternVar[MatchAny]()";
			}
		}
		
	};
	
	typedef std::vector<Node<PatternVar>> PatternVarList;
	
}

#endif
