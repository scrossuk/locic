#include <cassert>
#include <cstdio>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM/Context.hpp>
#include <locic/Support/SharedMaps.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Passes.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void Run(const SharedMaps& sharedMaps, const AST::NamespaceList& rootASTNamespaces, SEM::Context& semContext, Debug::Module& debugModule) {
			try {
				// Create 'context' to hold information about code structures.
				Context context(sharedMaps, debugModule, semContext);
				
				// Push root namespace on to the stack.
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semContext.rootNamespace()));
				
				// ---- Pass 1: Add namespaces, type names and template variables.
				AddGlobalStructuresPass(context, rootASTNamespaces);
				
				// ---- Pass 2: Add types of template variables.
				AddTemplateVariableTypesPass(context, rootASTNamespaces);
				
				// ---- Pass 3: Add alias values.
				AddAliasValuesPass(context, rootASTNamespaces);
				
				// ---- Pass 4: Add type member variables.
				AddTypeMemberVariablesPass(context, rootASTNamespaces);
				
				// ---- Pass 5: Create function declarations.
				AddFunctionDeclsPass(context, rootASTNamespaces);
				
				// ---- Pass 6: Complete type template variable requirements.
				CompleteTypeTemplateVariableRequirementsPass(context, rootASTNamespaces);
				
				// ---- Pass 7: Complete function template variable requirements.
				CompleteFunctionTemplateVariableRequirementsPass(context, rootASTNamespaces);
				
				// ---- Pass 8: Generate default methods.
				GenerateDefaultMethodsPass(context);
				
				// ---- Pass 9: Check all previous template instantiations are correct
				//              (all methods created by this point).
				CheckTemplateInstantiationsPass(context);
				
				// ---- Pass 10: Fill in function code.
				ConvertNamespace(context, rootASTNamespaces);
			} catch(const Exception& e) {
				printf("Semantic Analysis Error: %s\n", formatMessage(e.toString()).c_str());
				throw;
			}
		}
		
	}
	
}

