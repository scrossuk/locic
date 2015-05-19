#ifndef LOCIC_SEMANTICANALYSIS_CALLVALUE_HPP
#define LOCIC_SEMANTICANALYSIS_CALLVALUE_HPP

#include <locic/Support/HeapArray.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourceLocation;
		
	}
	
	namespace SEM {
		
		class Value;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		SEM::Value CallValue(Context& context, SEM::Value value, HeapArray<SEM::Value> args, const Debug::SourceLocation& location);
		
	}
	
}

#endif
