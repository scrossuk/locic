#ifndef LOCIC_SEMANTICANALYSIS_LVAL_HPP
#define LOCIC_SEMANTICANALYSIS_LVAL_HPP

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		const SEM::Type* makeValueLvalType(Context& context, const SEM::Type* valueType);
		
		// Wraps the given type in a default implicit lval, unless
		// the type is itself an lval (or some number of refs that lead
		// to an lval).
		const SEM::Type* makeLvalType(Context& context, bool isFinal, const SEM::Type* valueType);
		
		SEM::Value dissolveLval(Context& context, SEM::Value lvalValue, const Debug::SourceLocation& location);
		
		SEM::Value tryDissolveValue(Context& context, SEM::Value value, const Debug::SourceLocation& location);
		
	}
	
}

#endif
