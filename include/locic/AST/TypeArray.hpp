#ifndef LOCIC_AST_TYPEARRAY_HPP
#define LOCIC_AST_TYPEARRAY_HPP

#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace AST {
	
		class Type;
		
		constexpr size_t TypeArrayBaseSize = 8;
		using TypeArray = Array<const Type*, TypeArrayBaseSize>;
		
	}
	
}

#endif
