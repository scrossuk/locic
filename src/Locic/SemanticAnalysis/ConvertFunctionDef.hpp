#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDEF_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDEF_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Function * ConvertFunctionDef(Context& context, AST::Function* function, bool isMethod = false);
		
	}
	
}

#endif
