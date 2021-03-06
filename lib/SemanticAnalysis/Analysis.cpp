#include <cassert>
#include <cstdio>

#include <locic/AST.hpp>
#include <locic/AST/Context.hpp>
#include <locic/Debug.hpp>
#include <locic/Support/SharedMaps.hpp>

#include <locic/SemanticAnalysis/Analysis.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Passes.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void Run(const SharedMaps& sharedMaps, const AST::NamespaceList& rootASTNamespaces,
		         AST::Module& astModule, Debug::Module& debugModule, DiagnosticReceiver& diagReceiver) {
			try {
				// Create 'context' to hold information about code structures.
				Context context(sharedMaps, debugModule, astModule.context(),
				                diagReceiver);
				
				// Push root namespace on to the stack.
				PushScopeElement pushScopeElement(context.scopeStack(),
				                                  ScopeElement::Namespace(astModule.rootNamespace()));
				
				// ---- Add namespaces, type names and template variables.
				AddGlobalStructuresPass(context, rootASTNamespaces);
				
				// ---- Add types of template variables.
				AddTemplateVariableTypesPass(context, rootASTNamespaces);
				
				// ---- Add type member variables.
				AddTypeMemberVariablesPass(context, rootASTNamespaces);
				
				// ---- Create function declarations.
				AddFunctionDeclsPass(context, rootASTNamespaces);
				
				AddFunctionTypesPass(context, rootASTNamespaces);
				
				// ---- Complete type template variable requirements.
				CompleteTypeTemplateVariableRequirementsPass(context, rootASTNamespaces);
				
				// ---- Complete function template variable requirements.
				CompleteFunctionTemplateVariableRequirementsPass(context, rootASTNamespaces);
				
				// ---- Generate default methods.
				GenerateDefaultMethodsPass(context, rootASTNamespaces);
				
				// ---- Add alias values.
				AddAliasValuesPass(context, rootASTNamespaces);
				
				// ---- Check all previous template instantiations are correct
				//      (all methods created by this point).
				CheckTemplateInstantiationsPass(context);
				
				EvaluateStaticAssertsPass(context, rootASTNamespaces);
				
				// ---- Fill in function code.
				ConvertNamespace(context, rootASTNamespaces);
			} catch (const SkipException& e) {
				(void) e;
			}
		}
		
	}
	
}

