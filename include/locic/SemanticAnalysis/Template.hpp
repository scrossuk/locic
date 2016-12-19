#ifndef LOCIC_SEMANTICANALYSIS_TEMPLATE_HPP
#define LOCIC_SEMANTICANALYSIS_TEMPLATE_HPP

#include <vector>

#include <locic/AST.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void CheckTemplateInstantiation(Context& context,
		                                const AST::TemplatedObject& templatedObject,
		                                const AST::TemplateVarMap& variableAssignments,
		                                const Debug::SourceLocation& location);
		
		AST::TemplateVarMap GenerateTemplateVarMap(Context& context, const AST::TemplatedObject& templatedObject,
			SEM::ValueArray values, const Debug::SourceLocation& location,
			AST::TemplateVarMap variableAssignments = AST::TemplateVarMap());
		
		AST::TemplateVarMap GenerateSymbolTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
		
		SEM::ValueArray GetTemplateValues(const AST::TemplateVarMap& templateVarMap, const AST::TemplateVarArray& templateVariables);
		
		SEM::ValueArray makeTemplateArgs(Context& context, AST::TypeArray typeArray);
		
	}
	
}

#endif
