#ifndef LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP

#include <Locic/AST.hpp>
#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
		
		Map<SEM::TemplateVar*, SEM::Type*> GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
		
		std::vector<SEM::Type*> GetTemplateValues(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
	
		// Convert a type annotation to a semantic type definition.
		SEM::Type* ConvertType(Context& context, const AST::Node<AST::Type>& type);
		
	}
	
}

#endif
