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
	
		// Get all type names, and build initial type instance structures.
		bool AddTypeInstances(StructuralContext& context, AST::Module* astModule){
			//-- Initial phase: get all type names.
			for(std::size_t i = 0; i < astModule->typeInstances.size(); i++){
				AST::TypeInstance* astTypeInstance = astModule->typeInstances.at(i);
				SEM::TypeInstance * semTypeInstance =
					new SEM::TypeInstance((SEM::TypeInstance::TypeEnum) astTypeInstance->typeEnum,
						astTypeInstance->name);
				
				if(!context.addTypeInstance(astTypeInstance->name, semTypeInstance)){
					printf("Semantic Analysis Error: type already defined with name '%s'.\n", astTypeInstance->name.c_str());
					return false;
				}
			}
			return true;
		}
		
		// Fill in type instance structures with member variable information.
		bool AddTypeMemberVariables(StructuralContext& context, AST::Module * astModule){
			for(std::size_t i = 0; i < astModule->typeInstances.size(); i++){
				AST::TypeInstance* astTypeInstance = astModule->typeInstances.at(i);
				SEM::TypeInstance* semTypeInstance = context.getTypeInstance(astTypeInstance->name);
				assert(semTypeInstance != NULL);
				
				for(std::size_t j = 0; j < astTypeInstance->variables.size(); j++){
					AST::TypeVar * typeVar = astTypeInstance->variables.at(j);
					SEM::Type * semType = ConvertType(context, typeVar->type, SEM::Type::LVALUE);
					
					if(semType == NULL){
						printf("Semantic Analysis Error: invalid type for type instance member '%s'.\n", typeVar->name.c_str());
						return false;
					}
					
					semTypeInstance->variableNames.push_back(typeVar->name);
					SEM::Var * var = new SEM::Var(SEM::Var::MEMBER, j, semType, semTypeInstance);
					semTypeInstance->variables.push_back(var);
				}
			}
			
			return true;
		}
		
		bool AddFunctionDecls(StructuralContext& context, AST::Module* astModule){
			//-- Scan for functions.
			for(std::size_t i = 0; i < astModule->functions.size(); i++){
				AST::Function * astFunction = astModule->functions.at(i);
				
				if(!context.addFunction(astFunction->getFullName(), ConvertFunctionDecl(context, astFunction))) {
					printf("Semantic Analysis Error: function already defined with name '%s'.\n", astFunction->getFullName().c_str());
					return false;
				}
			}
			
			//-- Scan for class methods.
			for(std::size_t i = 0; i < astModule->typeInstances.size(); i++){
				AST::TypeInstance * astTypeInstance = astModule->typeInstances.at(i);
				
				for(std::size_t j = 0; j < astTypeInstance->functions.size(); j++){
					AST::Function * astFunction = astTypeInstance->functions.at(j);
					
					if(!context.addFunction(astFunction->getFullName(), ConvertFunctionDecl(context, astFunction))) {
						printf("Semantic Analysis Error: function already defined with name '%s'.\n", astFunction->getFullName().c_str());
						return false;
					}
				}
			}
			
			return true;
		}
	
		std::list<SEM::Module*> Run(const std::list<AST::Module*>& modules) {
			SEM::Namespace * rootNamespace = new SEM::Namespace("");
			GlobalContext globalContext(rootNamespace);
			
			// Convert type instance information.
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				if(!AddTypeInstances(globalContext, astModule)){
					return std::list<SEM::Module*>();
				}
			}
			
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				if(!AddTypeMemberVariables(globalContext, astModule)){
					return std::list<SEM::Module*>();
				}
			}
			
			// Convert function declaration information.
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				if(!AddFunctionDecls(globalContext, astModule)){
					return std::list<SEM::Module*>();
				}
			}
			
			// Convert the definitions of functions and class methods.
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

