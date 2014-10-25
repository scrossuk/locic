#ifndef LOCIC_SEMANTICANALYSIS_TEMPLATE_HPP
#define LOCIC_SEMANTICANALYSIS_TEMPLATE_HPP

#include <vector>

#include <locic/AST.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		void addRequireTypeInstance(Context& context, SEM::TemplateRequireMap& requireMap, SEM::TemplateVar* templateVar);
		
		void addTypeToRequirement(Context& context, SEM::TypeInstance* const requireInstance, const SEM::Type* const newType);
		
		const SEM::TypeInstance* getObjectOrSpecType(Context& context, const SEM::Type* type);
		
		TemplatedTypeInstance getTemplateTypeInstance(Context& context, const SEM::Type* type);
		
		SEM::TemplateVarMap GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
		
		std::vector<const SEM::Type*> GetTemplateValues(const SEM::TemplateVarMap& templateVarMap, const std::vector<SEM::TemplateVar*>& templateVariables);
		
		bool TemplateValueSatisfiesRequirement(const TemplatedTypeInstance& objectType, const TemplatedTypeInstance& requireType);
		
	}
	
}

#endif
