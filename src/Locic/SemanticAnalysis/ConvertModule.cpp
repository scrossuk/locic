#include <cstddef>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <Locic/SemanticAnalysis/ConvertModule.hpp>

namespace Locic{

namespace SemanticAnalysis{

bool ConvertModule(GlobalContext& globalContext, AST::Module* module, SEM::Module* semModule) {
	ModuleContext moduleContext(globalContext, semModule);
	
	// Build each function definition.
	std::list<AST::FunctionDef*>::const_iterator it;
	
	for(it = module->functionDefinitions.begin(); it != module->functionDefinitions.end(); ++it) {
		AST::FunctionDef* synFunctionDef = *it;
		
		SEM::FunctionDef* semFunctionDef = ConvertFunctionDef(moduleContext, synFunctionDef);
		
		if(semFunctionDef == NULL) {
			return false;
		}
		
		semModule->functionDefinitions.push_back(semFunctionDef);
	}
	
	return true;
}

}

}

