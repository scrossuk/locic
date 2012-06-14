#ifndef LOCIC_AST_TYPEVAR_HPP
#define LOCIC_AST_TYPEVAR_HPP

#include <string>
#include <Locic/AST/Type.hpp>

namespace AST {

	struct TypeVar {
		Type* type;
		std::string name;
		
		inline TypeVar(Type* t, const std::string& n)
			: type(t), name(n) { }
	};
	
}

#endif
