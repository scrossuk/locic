#ifndef LOCIC_SEMANTICANALYSIS_CONVERTMODULE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTMODULE_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool ConvertModule(Context& context, AST::Module* module, SEM::Module* semModule);
		
	}
	
}

#endif
