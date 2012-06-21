#ifndef LOCIC_SEM_VAR_HPP
#define LOCIC_SEM_VAR_HPP

#include <cstddef>

namespace SEM{

	struct Type;
	struct TypeInstance;

	struct Var{
		enum TypeEnum{
			LOCAL,
			PARAM,
			STRUCTMEMBER,
			THIS
		} typeEnum;
		
		std::size_t id;
		Type * type;
		TypeInstance * parent;
		
		inline Var(TypeEnum e, std::size_t i, Type * t, TypeInstance * p = NULL)
			: typeEnum(e), id(i), type(t), parent(p){ }
	};

}

#endif
