#ifndef LOCIC_SEMANTICANALYSIS_REF_HPP
#define LOCIC_SEMANTICANALYSIS_REF_HPP

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Type* getDerefType(SEM::Type* type);
		
		SEM::Value* derefValue(SEM::Value* value);
		
		SEM::Value* derefAll(SEM::Value* value);
		
	}
	
}

#endif
