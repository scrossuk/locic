#ifndef LOCIC_SEMANTICANALYSIS_METHODSETSATSIFIES_HPP
#define LOCIC_SEMANTICANALYSIS_METHODSETSATSIFIES_HPP

namespace locic {
	
	class OptionalDiag;
	
	namespace SemanticAnalysis {
		
		class Context;
		class MethodSet;
		
		OptionalDiag
		methodSetSatisfiesRequirement(Context& context,
		                              const MethodSet* checkSet,
		                              const MethodSet* requireSet);
		
	}
	
}

#endif
