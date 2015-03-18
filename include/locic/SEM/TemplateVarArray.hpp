#ifndef LOCIC_SEM_TEMPLATEVARARRAY_HPP
#define LOCIC_SEM_TEMPLATEVARARRAY_HPP

#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace SEM {
		
		class TemplateVar;
		
		constexpr size_t TemplateVarArrayBaseSize = 8;
		using TemplateVarArray = Array<TemplateVar*, TemplateVarArrayBaseSize>;
		
	}
	
}

#endif
