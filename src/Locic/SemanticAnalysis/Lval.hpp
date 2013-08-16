#ifndef LOCIC_SEMANTICANALYSIS_LVAL_HPP
#define LOCIC_SEMANTICANALYSIS_LVAL_HPP

#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Type* makeValueLvalType(Context& context, bool isLvalMutable, SEM::Type* valueType);
		
		SEM::Type* makeLvalType(Context& context, bool isCustomLval, bool isLvalMutable, SEM::Type* valueType);
		
	}
	
}

#endif
