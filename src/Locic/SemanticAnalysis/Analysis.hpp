#ifndef LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP
#define LOCIC_SEMANTICANALYSIS_ANALYSIS_HPP

#include <vector>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		std::vector<SEM::Module*> Run(const std::vector<AST::Module*>& modules);
		
	}
	
}

#endif
