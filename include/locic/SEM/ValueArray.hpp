#ifndef LOCIC_SEM_VALUEARRAY_HPP
#define LOCIC_SEM_VALUEARRAY_HPP

#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace SEM {
		
		class Value;
		
		constexpr size_t ValueArrayBaseSize = 8;
		using ValueArray = Array<Value, ValueArrayBaseSize>;
		
	}
	
}

#endif
