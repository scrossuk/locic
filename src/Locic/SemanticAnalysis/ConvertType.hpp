#ifndef LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		// Convert a type annotation to a semantic type definition.
		SEM::Type* ConvertType(TypeInfoContext& context, AST::Type* type, bool isLValue);
		
	}
	
}

#endif
