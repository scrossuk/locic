#ifndef LOCIC_SEMANTICANALYSIS_TEMPLATE_HPP
#define LOCIC_SEMANTICANALYSIS_TEMPLATE_HPP

#include <vector>

#include <locic/AST.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::TemplateVarMap GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
		
		std::vector<const SEM::Type*> GetTemplateValues(const SEM::TemplateVarMap& templateVarMap, const std::vector<SEM::TemplateVar*>& templateVariables);
		
	}
	
}

#endif
