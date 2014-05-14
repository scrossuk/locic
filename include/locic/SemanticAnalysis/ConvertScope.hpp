#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		bool WillScopeReturn(const SEM::Scope& scope);
		
		bool CanScopeThrow(const SEM::Scope& scope);
		
		SEM::Scope* ConvertScope(Context& context, AST::Node<AST::Scope> astScope);
		
	}
	
}

#endif
