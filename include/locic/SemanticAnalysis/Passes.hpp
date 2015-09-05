#ifndef LOCIC_SEMANTICANALYSIS_PASSES_HPP
#define LOCIC_SEMANTICANALYSIS_PASSES_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		class Context;
		
		/**
		 * \brief Add global structures.
		 * 
		 * This pass builds the initial SEM tree structure by adding
		 * namespaces, type instances and aliases.
		 */
		void AddGlobalStructuresPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
				
		/**
		 * \brief Add template variable types.
		 */
		void AddTemplateVariableTypesPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
		/**
		 * \brief Add alias values.
		 */
		void AddAliasValuesPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
		/**
		 * \brief Add type member variables.
		 */
		void AddTypeMemberVariablesPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
		/**
		 * \brief Add function declarations.
		 */
		void AddFunctionDeclsPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
		/**
		 * \brief Add function types.
		 */
		void AddFunctionTypesPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
		/**
		 * \brief Complete type template variable requirements.
		 */
		void CompleteTypeTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
		/**
		 * \brief Complete function template variable requirements.
		 */
		void CompleteFunctionTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
		/**
		 * \brief Generate default methods.
		 */
		void GenerateDefaultMethodsPass(Context& context);
		
		/**
		 * \brief Check template instantiations.
		 */
		void CheckTemplateInstantiationsPass(Context& context);
		
		/**
		 * \brief Check all the static asserts resolve to true.
		 */
		 void EvaluateStaticAssertsPass(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
	}
	
}

#endif
