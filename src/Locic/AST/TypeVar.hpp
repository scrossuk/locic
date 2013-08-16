#ifndef LOCIC_AST_TYPEVAR_HPP
#define LOCIC_AST_TYPEVAR_HPP

#include <string>
#include <Locic/AST/Type.hpp>

namespace AST {

	struct TypeVar {
		Type* type;
		std::string name;
		bool usesCustomLval;
		
		inline TypeVar(Type* t, const std::string& n, bool u)
			: type(t), name(n), usesCustomLval(u) { }
	};
	
}

#endif
