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
		 * verify an AST, reporting any errors/warnings/notes as it goes
		 * along, and manipulate it to be useful for CodeGen.
		 * 
		 * TODO: error handling needs to be improved!
		 */
		void Run(const SharedMaps& sharedMaps, const AST::NamespaceList& rootASTNamespaces,
		         AST::Module& astModule, Debug::Module& debugModule, DiagnosticReceiver& diagReceiver);
		
	}
	
}

#endif
