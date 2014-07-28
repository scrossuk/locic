#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Scope* ConvertScope(Context& context, const AST::Node<AST::Scope>& astScopeNode);
		
	}
	
}

#endif
