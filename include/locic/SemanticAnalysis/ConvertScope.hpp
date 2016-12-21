#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP

#include <memory>

#include <locic/AST.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void ConvertScope(Context& context, AST::Node<AST::Scope>& astScopeNode);
		
	}
	
}

#endif
