#ifndef LOCIC_AST_VALUEARRAY_HPP
#define LOCIC_AST_VALUEARRAY_HPP

#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace AST {
		
		class Value;
		
		constexpr size_t ValueArrayBaseSize = 8;
		using ValueArray = Array<Value, ValueArrayBaseSize>;
		
	}
	
}

#endif
