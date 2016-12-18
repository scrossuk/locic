#ifndef LOCIC_AST_TEMPLATEVARARRAY_HPP
#define LOCIC_AST_TEMPLATEVARARRAY_HPP

#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace AST {
		
		class TemplateVar;
		
		constexpr size_t TemplateVarArrayBaseSize = 8;
		using TemplateVarArray = Array<TemplateVar*, TemplateVarArrayBaseSize>;
		
	}
	
}

#endif
