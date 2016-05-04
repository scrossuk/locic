#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFORLOOP_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTFORLOOP_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		std::unique_ptr<SEM::Scope>
		ConvertForLoop(Context& context, AST::Node<AST::Var>& astVarNode,
		               const AST::Node<AST::Value>& astInitValueNode,
		               const AST::Node<AST::Scope>& astScopeNode);
		
	}
	
}

#endif
