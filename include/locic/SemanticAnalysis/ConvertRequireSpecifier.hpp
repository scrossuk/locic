#ifndef LOCIC_SEMANTICANALYSIS_CONVERTREQUIRESPECIFIER_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTREQUIRESPECIFIER_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void ConvertRequireSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode, SEM::TemplateRequireMap& requireMap);
		
	}
	
}

#endif
