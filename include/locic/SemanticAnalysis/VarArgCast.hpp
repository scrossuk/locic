#ifndef LOCIC_SEMANTICANALYSIS_VARARGCAST_HPP
#define LOCIC_SEMANTICANALYSIS_VARARGCAST_HPP

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		bool isValidVarArgType(Context& context, const AST::Type* type);
		
		SEM::Value VarArgCast(Context& context, SEM::Value value, const Debug::SourceLocation& location);
		
	}
	
}

#endif
