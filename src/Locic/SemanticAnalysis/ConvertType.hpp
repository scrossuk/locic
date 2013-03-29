#ifndef LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP

#include <Locic/AST.hpp>
#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
		
		Map<SEM::TemplateVar*, SEM::Type*> GenerateTemplateVarMap(Context& context, const AST::Symbol& symbol);
	
		// Convert a type annotation to a semantic type definition.
		SEM::Type* ConvertType(Context& context, AST::Type* type, bool isLValue);
		
	}
	
}

#endif
