#ifndef LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTTYPE_HPP

#include <vector>

#include <locic/AST.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {
	
	namespace SEM {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		SEM::TemplateVarMap GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbolNode);
		
		SEM::TemplateVarMap GenerateBasicTemplateVarMap(Context& context, const AST::Node<AST::TypeList>& astTypeListNode);
		
		/**
		 * \brief Convert symbol to semantic object type.
		 */
		const SEM::Type* ConvertObjectType(Context& context, const AST::Node<AST::Symbol>& symbol);
		
		/**
		 * \brief Convert AST type annotation to a semantic type definition.
		 */
		const SEM::Type* ConvertType(Context& context, const AST::Node<AST::Type>& type);
		
	}
	
}

#endif
