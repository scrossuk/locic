#ifndef LOCIC_SEMANTICANALYSIS_VARARGCAST_HPP
#define LOCIC_SEMANTICANALYSIS_VARARGCAST_HPP

#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool isValidVarArgType(SEM::Type* type);
		
		SEM::Value* VarArgCast(SEM::Value* value);
		
	}
	
}

#endif
