#ifndef LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP

#include <locic/AST.hpp>
#include <locic/Map.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		Map<SEM::TemplateVar*, SEM::Type*> GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
		
		std::vector<SEM::Type*> GetTemplateValues(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
		
		/**
		 * \brief Convert symbol to semantic object type.
		 */
		SEM::Type* ConvertObjectType(Context& context, const AST::Node<AST::Symbol>& symbol);
		
		/**
		 * \brief Convert AST type annotation to a semantic type definition.
		 */
		SEM::Type* ConvertType(Context& context, const AST::Node<AST::Type>& type);
		
	}
	
}

#endif
