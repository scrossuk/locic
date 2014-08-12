#ifndef LOCIC_SEMANTICANALYSIS_CONVERTTYPEINSTANCE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTTYPEINSTANCE_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void ConvertTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode);
		
	}
	
}

#endif
