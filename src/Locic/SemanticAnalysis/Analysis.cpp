#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <Locic/SemanticAnalysis/ConvertModule.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		std::list<SEM::Module*> Run(const std::list<AST::Module*>& modules) {
			GlobalContext globalContext;
			
			std::vector< std::pair<AST::Struct *, SEM::TypeInstance *> > structPairs;
			
			//-- Initial phase: get all type names.
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				
				// Look for structures.
				for(std::list<AST::Struct*>::iterator sIt = astModule->structs.begin(); sIt != astModule->structs.end(); ++sIt) {
					AST::Struct* astStruct = *sIt;
					SEM::TypeInstance * semStruct = SEM::TypeInstance::Struct(astStruct->name);
					
					if(!globalContext.addTypeInstance(astStruct->name, semStruct)) {
						printf("Semantic Analysis Error: type already defined with name '%s'.\n", astStruct->name.c_str());
						return std::list<SEM::Module*>();
					}
					
					structPairs.push_back(std::make_pair(astStruct, semStruct));
				}
			}
			
			//-- Type instance phase: fill in data members of type instances.
			for(std::size_t i = 0; i < structPairs.size(); i++) {
				AST::Struct* astStruct = structPairs.at(i).first;
				SEM::TypeInstance* semStruct = structPairs.at(i).second;
				
				std::size_t id = 0;
				std::list<AST::TypeVar *>::const_iterator it;
				for(it = astStruct->variables.begin(); it != astStruct->variables.end(); ++it){
					AST::TypeVar * typeVar = *it;
					SEM::Type * semType = ConvertType(globalContext, typeVar->type, SEM::Type::LVALUE);
					
					if(semType == NULL){
						printf("Semantic Analysis Error: invalid type for struct member '%s'.\n", typeVar->name.c_str());
						return std::list<SEM::Module*>();
					}
					
					SEM::Var * var = new SEM::Var(SEM::Var::STRUCTMEMBER, id++, semType, semStruct);
					semStruct->variables.push_back(var);
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

