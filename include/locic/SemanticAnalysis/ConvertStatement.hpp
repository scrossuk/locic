#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSTATEMENT_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTSTATEMENT_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		bool WillStatementReturn(SEM::Statement* statement);
		
		bool CanStatementThrow(SEM::Statement* statement);
		
		SEM::Statement* ConvertStatement(Context& context, const AST::Node<AST::Statement>& statement);
		
	}
	
}

#endif
