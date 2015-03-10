#ifndef LOCIC_SEM_TYPEARRAY_HPP
#define LOCIC_SEM_TYPEARRAY_HPP

#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace SEM {
	
		class Type;
		
		constexpr size_t TypeArrayBaseSize = 8;
		using TypeArray = Array<const Type*, TypeArrayBaseSize>;
		
	}
	
}

#endif
