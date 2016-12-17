#ifndef LOCIC_SEM_TEMPLATEVARARRAY_HPP
#define LOCIC_SEM_TEMPLATEVARARRAY_HPP

#include <locic/Support/Array.hpp>

namespace locic {
		
	namespace AST {
		
		class TemplateVar;
		
	}
	
	namespace SEM {
		
		constexpr size_t TemplateVarArrayBaseSize = 8;
		using TemplateVarArray = Array<AST::TemplateVar*, TemplateVarArrayBaseSize>;
		
	}
	
}

#endif
