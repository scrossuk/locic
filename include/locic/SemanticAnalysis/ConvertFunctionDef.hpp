#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDEF_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDEF_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void ConvertFunctionDef(Context& context, const AST::Node<AST::Function>& astFunctionNode);
		
	}
	
}

#endif
