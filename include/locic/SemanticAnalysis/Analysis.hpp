#ifndef LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP
#define LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP

#include <vector>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Namespace* Run(const AST::NamespaceList& rootASTNamespaces, Debug::Module& debugModule);
		
	}
	
}

#endif
