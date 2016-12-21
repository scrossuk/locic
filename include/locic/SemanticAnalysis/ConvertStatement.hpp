#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSTATEMENT_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTSTATEMENT_HPP

#include <locic/AST.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		AST::Statement ConvertStatement(Context& context, const AST::Node<AST::StatementDecl>& statement);
		
	}
	
}

#endif
