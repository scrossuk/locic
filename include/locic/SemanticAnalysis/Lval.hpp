#ifndef LOCIC_SEMANTICANALYSIS_LVAL_HPP
#define LOCIC_SEMANTICANALYSIS_LVAL_HPP

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Type* makeValueLvalType(Context& context, bool isLvalMutable, SEM::Type* valueType);
		
		SEM::Type* makeLvalType(Context& context, bool usesCustomLval, bool isLvalMutable, SEM::Type* valueType);
		
		bool canDissolveValue(SEM::Value* value);
		
		SEM::Value* dissolveLval(SEM::Value* lvalValue);
		
		SEM::Value* tryDissolveValue(SEM::Value* value);
		
	}
	
}

#endif
