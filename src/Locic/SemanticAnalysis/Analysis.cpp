#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <Locic/SemanticAnalysis/ConvertModule.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		std::list<SEM::Module*> Run(const std::list<AST::Module*>& modules) {
			GlobalContext globalContext;
			
			//-- Initial phase: get all type names.
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				
				// Look for structures.
				for(std::list<AST::Struct*>::iterator sIt = astModule->structs.begin(); sIt != astModule->structs.end(); ++sIt) {
					AST::Struct* astStruct = *sIt;
					
					if(!globalContext.addTypeInstance(astStruct->name, SEM::TypeInstance::Struct(astStruct->name))) {
						printf("Semantic Analysis Error: type already defined with name '%s'.\n", astStruct->name.c_str());
						return std::list<SEM::Module*>();
					}
				}
			}
			
			//-- Declaration phase: scan for function and class method declarations (so they can be referenced by the final phase).
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				
				// Look for function declarations.
				for(std::list<AST::FunctionDecl*>::iterator it = astModule->functionDeclarations.begin(); it != astModule->functionDeclarations.end(); ++it) {
					AST::FunctionDecl* astFunctionDecl = *it;
					
					if(!globalContext.addFunctionDecl(astFunctionDecl->name, ConvertFunctionDecl(globalContext, astFunctionDecl))) {
						printf("Semantic Analysis Error: function already defined with name '%s'.\n", astFunctionDecl->name.c_str());
						return std::list<SEM::Module*>();
					}
				}
				
				// Look for function definitions (and grab their implicit 'declaration').
				for(std::list<AST::FunctionDef*>::iterator it = astModule->functionDefinitions.begin(); it != astModule->functionDefinitions.end(); ++it) {
					AST::FunctionDecl* astFunctionDecl = (*it)->declaration;
					
					if(!globalContext.addFunctionDecl(astFunctionDecl->name, ConvertFunctionDecl(globalContext, astFunctionDecl))) {
						printf("Semantic Analysis Error: function already defined with name '%s'.\n", astFunctionDecl->name.c_str());
						return std::list<SEM::Module*>();
					}
				}
			}
			
			//-- Definition phase: extend the semantic structure with the definitions of functions and class methods.
			std::list<SEM::Module*> semModules;
			
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				SEM::Module* semModule = new SEM::Module(astModule->name);
				
				if(!ConvertModule(globalContext, astModule, semModule)) {
					return std::list<SEM::Module*>();
				}
				
				semModules.push_back(semModule);
			}
			
			return semModules;
		}
		
	}
	
}

