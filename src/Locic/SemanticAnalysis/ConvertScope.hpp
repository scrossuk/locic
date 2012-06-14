#ifndef LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTSCOPE_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic{

	namespace SemanticAnalysis{

		bool WillScopeReturn(SEM_Scope * scope);

		SEM::Scope * ConvertScope(LocalContext& context, AST_Scope * scope);
		
	}
	
}

#endif
