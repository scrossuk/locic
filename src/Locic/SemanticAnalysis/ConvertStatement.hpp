#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSTATEMENT_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTSTATEMENT_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool WillStatementReturn(SEM::Statement* statement);
		
		SEM::Statement* ConvertStatement(Context& context, AST::Statement* statement);
		
	}
	
}

#endif
