#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFORLOOP_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTFORLOOP_HPP

#include <locic/AST.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		AST::Node<AST::Scope>
		ConvertForLoop(Context& context, AST::Node<AST::Var>& astVarNode,
		               const AST::Node<AST::ValueDecl>& astInitValueNode,
		               AST::Node<AST::Scope>& astScopeNode);
		
	}
	
}

#endif
