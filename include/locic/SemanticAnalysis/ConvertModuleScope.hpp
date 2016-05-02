#ifndef LOCIC_SEMANTICANALYSIS_CONVERTMODULESCOPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTMODULESCOPE_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		AST::ModuleScope
		ConvertModuleScope(const AST::Node<AST::ModuleScopeDecl>& astModuleScopeNode);
		
	}
	
}

#endif
