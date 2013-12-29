#ifndef LOCIC_SEMANTICANALYSIS_REF_HPP
#define LOCIC_SEMANTICANALYSIS_REF_HPP

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		size_t getRefCount(SEM::Type* type);
		
		SEM::Type* getDerefType(SEM::Type* type);
		
		SEM::Value* derefOne(SEM::Value* value);
		
		SEM::Value* derefValue(SEM::Value* value);
		
		SEM::Value* derefAll(SEM::Value* value);
		
	}
	
}

#endif
