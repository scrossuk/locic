#ifndef LOCIC_SEMANTICANALYSIS_METHODSETSATSIFIES_HPP
#define LOCIC_SEMANTICANALYSIS_METHODSETSATSIFIES_HPP

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class MethodSet;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		OptionalDiag
		methodSetSatisfiesRequirement(Context& context,
		                              const AST::MethodSet* checkSet,
		                              const AST::MethodSet* requireSet);
		
	}
	
}

#endif
