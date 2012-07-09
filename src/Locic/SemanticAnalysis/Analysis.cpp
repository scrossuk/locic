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
			
			std::vector< std::pair<AST::TypeInstance *, SEM::TypeInstance *> > typeInstancePairs;
			
			//-- Initial phase: get all type names.
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				
				for(std::size_t i = 0; i < astModule->typeInstances.size(); i++){
					AST::TypeInstance* astTypeInstance = astModule->typeInstances.at(i);
					SEM::TypeInstance * semTypeInstance =
						new SEM::TypeInstance((SEM::TypeInstance::TypeEnum) astTypeInstance->typeEnum,
							astTypeInstance->name);
					
					if(!globalContext.addTypeInstance(astTypeInstance->name, semTypeInstance)) {
						printf("Semantic Analysis Error: type already defined with name '%s'.\n", astTypeInstance->name.c_str());
						return std::list<SEM::Module*>();
					}
					
					typeInstancePairs.push_back(std::make_pair(astTypeInstance, semTypeInstance));
				}
			}
			
			//-- Type instance phase: fill in data members of type instances.
			for(std::size_t i = 0; i < typeInstancePairs.size(); i++) {
				AST::TypeInstance* astTypeInstance = typeInstancePairs.at(i).first;
				SEM::TypeInstance * semTypeInstance = typeInstancePairs.at(i).second;
			
				for(std::size_t i = 0; i < astTypeInstance->variables.size(); i++){
					AST::TypeVar * typeVar = astTypeInstance->variables.at(i);
					SEM::Type * semType = ConvertType(globalContext, typeVar->type, SEM::Type::LVALUE);
					
					if(semType == NULL){
						printf("Semantic Analysis Error: invalid type for type instance member '%s'.\n", typeVar->name.c_str());
						return std::list<SEM::Module*>();
					}
					
					semTypeInstance->variableNames.push_back(typeVar->name);
					SEM::Var * var = new SEM::Var(SEM::Var::MEMBER, i, semType, semTypeInstance);
					semTypeInstance->variables.push_back(var);
				}
			}
			
			//-- Declaration phase: scan for functions and class methods (so they can be referenced by the final phase).
			for(std::list<AST::Module*>::const_iterator it = modules.begin(); it != modules.end(); ++it) {
				AST::Module* astModule = *it;
				
				for(std::size_t i = 0; i < astModule->functions.size(); i++){
					AST::Function * astFunction = astModule->functions.at(i);
					
					if(!globalContext.addFunction(astFunction->getFullName(), ConvertFunctionDecl(globalContext, astFunction))) {
						printf("Semantic Analysis Error: function already defined with name '%s'.\n", astFunction->name.c_str());
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

