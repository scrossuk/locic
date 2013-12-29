#ifndef LOCIC_SEMANTICANALYSIS_LVAL_HPP
#define LOCIC_SEMANTICANALYSIS_LVAL_HPP

#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		// Wraps the given type as lval<type> value_type<type>.
		SEM::Type* makeValueLvalType(Context& context, bool isLvalMutable, SEM::Type* valueType);
		
		// Wraps the given type as lval<type> value_type<type>, unless
		// the type is itself an lval (or some number of refs that lead
		// to an lval).
		SEM::Type* makeLvalType(Context& context, bool isLvalConst, SEM::Type* valueType);
		
		size_t getLvalCount(SEM::Type* type);
		
		bool canDissolveValue(SEM::Value* value);
		
		SEM::Value* dissolveLval(SEM::Value* lvalValue);
		
		SEM::Value* tryDissolveValue(SEM::Value* value);
		
	}
	
}

#endif
