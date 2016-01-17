#ifndef LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP
#define LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class DiagnosticReceiver;
	class SharedMaps;
	
	namespace Debug {
		
		class Module;
		
	}
	
	namespace SEM {
		
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
		         SEM::Module& semModule, Debug::Module& debugModule, DiagnosticReceiver& diagReceiver);
		
	}
	
}

#endif
