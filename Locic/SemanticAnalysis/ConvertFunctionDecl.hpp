#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDECL_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDECL_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Function* ConvertFunctionDecl(Context& context, AST::Node<AST::Function> function);
		
	}
	
}

#endif
