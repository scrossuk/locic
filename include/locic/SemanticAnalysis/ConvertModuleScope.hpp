#ifndef LOCIC_SEMANTICANALYSIS_CONVERTMODULESCOPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTMODULESCOPE_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace SEM {
		
		class ModuleScope;
		
	}
	
	namespace SemanticAnalysis {
		
		SEM::ModuleScope ConvertModuleScope(const AST::Node<AST::ModuleScope>& astModuleScopeNode);
		
	}
	
}

#endif
