#ifndef LOCIC_SEMANTICANALYSIS_VARARGCAST_HPP
#define LOCIC_SEMANTICANALYSIS_VARARGCAST_HPP

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		bool isValidVarArgType(SEM::Type* type);
		
		SEM::Value* VarArgCast(SEM::Value* value, const Debug::SourceLocation& location);
		
	}
	
}

#endif
