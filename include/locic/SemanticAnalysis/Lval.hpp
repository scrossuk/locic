#ifndef LOCIC_SEMANTICANALYSIS_LVAL_HPP
#define LOCIC_SEMANTICANALYSIS_LVAL_HPP

#include <locic/Debug.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		const AST::Type* makeValueLvalType(Context& context, const AST::Type* valueType);
		
		// Wraps the given type in a default implicit lval, unless
		// the type is itself an lval (or some number of refs that lead
		// to an lval).
		const AST::Type* makeLvalType(Context& context, bool isFinal, const AST::Type* valueType);
		
		AST::Value dissolveLval(Context& context, AST::Value lvalValue, const Debug::SourceLocation& location);
		
		AST::Value tryDissolveValue(Context& context, AST::Value value, const Debug::SourceLocation& location);
		
	}
	
}

#endif
