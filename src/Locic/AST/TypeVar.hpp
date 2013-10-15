#ifndef LOCIC_AST_TYPEVAR_HPP
#define LOCIC_AST_TYPEVAR_HPP

#include <string>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/Type.hpp>

namespace AST {

	struct TypeVar {
		Node<Type> type;
		std::string name;
		bool usesCustomLval;
		
		inline TypeVar(Node<Type> t, const std::string& n, bool u)
			: type(t), name(n), usesCustomLval(u) { }
	};
	
	typedef std::vector<Node<TypeVar>> TypeVarList;
	
}

#endif
