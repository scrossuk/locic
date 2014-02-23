#ifndef LOCIC_SEMANTICANALYSIS_CONVERTEXCEPTION_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTEXCEPTION_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Function* CreateExceptionConstructor(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode, SEM::TypeInstance* semTypeInstance);
		
	}
	
}

#endif
