#ifndef LOCIC_SEMANTICANALYSIS_METHODSETSATSIFIES_HPP
#define LOCIC_SEMANTICANALYSIS_METHODSETSATSIFIES_HPP

namespace locic {
	
	namespace SemanticAnalysis {
		
		class Context;
		class MethodSet;
		
		bool methodSetSatisfiesRequirement(Context& context,
		                                   const MethodSet* checkSet,
		                                   const MethodSet* requireSet);
		
	}
	
}

#endif
