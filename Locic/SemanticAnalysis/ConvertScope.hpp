#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool WillScopeReturn(const SEM::Scope& scope);
		
		SEM::Scope* ConvertScope(Context& context, AST::Node<AST::Scope> astScope);
		
	}
	
}

#endif
