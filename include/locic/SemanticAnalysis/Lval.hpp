#ifndef LOCIC_SEMANTICANALYSIS_LVAL_HPP
#define LOCIC_SEMANTICANALYSIS_LVAL_HPP

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		const SEM::Type* makeValueLvalType(Context& context, const SEM::Type* valueType);
		
		const SEM::Type* makeMemberLvalType(Context& context, const SEM::Type* valueType);
		
		// Wraps the given type in a default implicit lval, unless
		// the type is itself an lval (or some number of refs that lead
		// to an lval).
		const SEM::Type* makeLvalType(Context& context, bool isMember, bool isFinal, const SEM::Type* valueType);
		
		size_t getLvalCount(const SEM::Type* type);
		
		bool canDissolveType(Context& context, const SEM::Type* const rawType);
		
		bool canDissolveValue(Context& context, const SEM::Value& value);
		
		SEM::Value dissolveLval(Context& context, SEM::Value lvalValue, const Debug::SourceLocation& location);
		
		SEM::Value tryDissolveValue(Context& context, SEM::Value value, const Debug::SourceLocation& location);
		
	}
	
}

#endif
