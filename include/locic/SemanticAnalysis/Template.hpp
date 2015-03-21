#ifndef LOCIC_SEMANTICANALYSIS_TEMPLATE_HPP
#define LOCIC_SEMANTICANALYSIS_TEMPLATE_HPP

#include <vector>

#include <locic/AST.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		SEM::TemplateVarMap GenerateTemplateVarMap(Context& context, const SEM::TemplatedObject& templatedObject,
			SEM::ValueArray values, const Debug::SourceLocation& location,
			SEM::TemplateVarMap variableAssignments = SEM::TemplateVarMap());
		
		SEM::TemplateVarMap GenerateSymbolTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
		
		SEM::ValueArray GetTemplateValues(const SEM::TemplateVarMap& templateVarMap, const SEM::TemplateVarArray& templateVariables);
		
		SEM::ValueArray makeTemplateArgs(Context& context, SEM::TypeArray typeArray);
		
	}
	
}

#endif
