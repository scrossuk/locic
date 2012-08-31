#ifndef LOCIC_SEMANTICANALYSIS_CONVERTCLASSDEF_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTCLASSDEF_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::TypeInstance * ConvertClassDef(Context& context, AST::TypeInstance* typeInstance);
		
	}
	
}

#endif
