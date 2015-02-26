#ifndef LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP
#define LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP

#include <vector>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

namespace locic {
	
	class StringHost;
	
	namespace SemanticAnalysis {
	
		void Run(const StringHost& stringHost, const AST::NamespaceList& rootASTNamespaces, SEM::Context& semContext, Debug::Module& debugModule);
		
	}
	
}

#endif
