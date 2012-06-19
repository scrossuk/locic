#ifndef LOCIC_SEM_VAR_HPP
#define LOCIC_SEM_VAR_HPP

#include <cstddef>

namespace SEM{

	struct Type;

	struct Var{
		enum TypeEnum{
			LOCAL,
			PARAM,
			THIS
		} typeEnum;
		
		std::size_t id;
		Type * type;
		
		inline Var(TypeEnum e, std::size_t i, Type * t)
			: typeEnum(e), id(i), type(t){ }
	};

}

#endif
