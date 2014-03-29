#ifndef LOCIC_SEMANTICANALYSIS_LVAL_HPP
#define LOCIC_SEMANTICANALYSIS_LVAL_HPP

#include <locic/Debug.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Type* makeValueLvalType(Context& context, bool isLvalConst, SEM::Type* valueType);
		
		SEM::Type* makeMemberLvalType(Context& context, bool isLvalConst, SEM::Type* valueType);
		
		// Wraps the given type in a default implicit lval, unless
		// the type is itself an lval (or some number of refs that lead
		// to an lval).
		SEM::Type* makeLvalType(Context& context, bool isMember, bool isLvalConst, SEM::Type* valueType);
		
		size_t getLvalCount(SEM::Type* type);
		
		bool canDissolveValue(SEM::Value* value);
		
		SEM::Value* dissolveLval(SEM::Value* lvalValue, const Debug::SourceLocation& location);
		
		SEM::Value* tryDissolveValue(SEM::Value* value, const Debug::SourceLocation& location);
		
	}
	
}

#endif
