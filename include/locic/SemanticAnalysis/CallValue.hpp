#ifndef LOCIC_SEMANTICANALYSIS_CALLVALUE_HPP
#define LOCIC_SEMANTICANALYSIS_CALLVALUE_HPP

#include <locic/Support/HeapArray.hpp>

namespace locic {
	
	namespace AST {
		
		class Value;
		
	}
	
	namespace Debug {
		
		class SourceLocation;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		AST::Value CallValue(Context& context, AST::Value value, HeapArray<AST::Value> args, const Debug::SourceLocation& location);
		
	}
	
}

#endif
