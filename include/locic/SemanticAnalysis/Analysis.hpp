#ifndef LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP
#define LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class DiagnosticReceiver;
	class SharedMaps;
	
	namespace AST {
		
		class Module;
		
	}
	
	namespace Debug {
		
		class Module;
		
	}
	
	namespace SemanticAnalysis {
		
		/**
		 * \brief Run Semantic Analysis passes
		 * 
		 * This function runs all the Semantic Analysis passes to
		 * convert an AST tree into a SEM tree. In the process of doing
		 * this it detects errors in the AST tree, which are reported.
		 * 
		 * TODO: error handling needs to be improved!
		 */
		void Run(const SharedMaps& sharedMaps, const AST::NamespaceList& rootASTNamespaces,
		         AST::Module& semModule, Debug::Module& debugModule, DiagnosticReceiver& diagReceiver);
		
	}
	
}

#endif
