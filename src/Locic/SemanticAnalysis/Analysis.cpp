#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
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
						context.getName() + astTypeInstance->name);
				
				if(!context.addTypeInstance(astTypeInstance->name, semTypeInstance)){
					printf("Semantic Analysis Error: type already defined with name '%s'.\n", astTypeInstance->name.c_str());
					return false;
				}
			}
			return true;
		}
		
		// Fill in type instance structures with member variable information.
		bool AddTypeMemberVariables(StructuralContext& context, AST::TypeInstance * astTypeInstance){
			SEM::TypeInstance* semTypeInstance = context.getTypeInstance(context.getName() + astTypeInstance->name);
			assert(semTypeInstance != NULL);
			
			for(std::size_t i = 0; i < astTypeInstance->variables.size(); i++){
				AST::TypeVar * typeVar = astTypeInstance->variables.at(i);
				SEM::Type * semType = ConvertType(context, typeVar->type, SEM::Type::LVALUE);
				
				if(semType == NULL){
					printf("Semantic Analysis Error: invalid type for type instance member '%s'.\n", typeVar->name.c_str());
					return false;
				}
				
				SEM::Var * var = new SEM::Var(SEM::Var::MEMBER, i, semType, semTypeInstance);
				if(!semTypeInstance->variables.tryInsert(typeVar->name, var)){
					printf("Semantic Analysis Error: more than one member variable shares name '%s'.\n", typeVar->name.c_str());
					return false;
				}
			}
			
			return true;
		}
		
		bool AddTypeMemberVariables(StructuralContext& context, AST::Module * astModule, SEM::Module * semModule){
			ModuleContext moduleContext(context, semModule);
		
			for(std::size_t i = 0; i < astModule->typeInstances.size(); i++){
				AST::TypeInstance* astTypeInstance = astModule->typeInstances.at(i);
				if(!AddTypeMemberVariables(moduleContext, astTypeInstance)) return false;
			}
			
			return true;
		}
		
		bool AddFunctionDecls(StructuralContext& context, AST::Function* astFunction, bool isMethod = false){
			SEM::Function * semFunction = ConvertFunctionDecl(context, astFunction, isMethod);
			assert(semFunction != NULL);
			
			SEM::Function * existingFunction = context.getFunction(context.getName() + astFunction->name, false);
			if(existingFunction != NULL){
				if(!AreTypesEqual(semFunction->type, existingFunction->type)){
					printf("Semantic Analysis Error: declarations of function '%s' don't match.\n", astFunction->getFullName().c_str());
					return false;
				}
				return true;
			}
				
			if(!context.addFunction(astFunction->name, semFunction, isMethod)) {
				printf("Semantic Analysis Error: function name '%s' clashes with existing type name.\n", astFunction->getFullName().c_str());
				return false;
			}
			
			return true;
		}
		
		bool AddFunctionDecls(StructuralContext& context, AST::TypeInstance* astTypeInstance){
			SEM::TypeInstance* semTypeInstance = context.getTypeInstance(context.getName() + astTypeInstance->name);
			assert(semTypeInstance != NULL);
			
			TypeInstanceContext typeInstanceContext(context, semTypeInstance);
			
			for(std::size_t i = 0; i < astTypeInstance->constructors.size(); i++){
				AST::Function * astFunction = astTypeInstance->constructors.at(i);
				if(!AddFunctionDecls(typeInstanceContext, astFunction)) return false;
			}
		
			for(std::size_t i = 0; i < astTypeInstance->functions.size(); i++){
				AST::Function * astFunction = astTypeInstance->functions.at(i);
				if(!AddFunctionDecls(typeInstanceContext, astFunction, true)) return false;
			}
			
			return true;
		}
		
		bool AddFunctionDecls(StructuralContext& context, AST::Module* astModule, SEM::Module* semModule){
			ModuleContext moduleContext(context, semModule);
			
			for(std::size_t i = 0; i < astModule->functions.size(); i++){
				AST::Function * astFunction = astModule->functions.at(i);
				if(!AddFunctionDecls(moduleContext, astFunction)) return false;
			}
			
			for(std::size_t i = 0; i < astModule->typeInstances.size(); i++){
				AST::TypeInstance * astTypeInstance = astModule->typeInstances.at(i);
				if(!AddFunctionDecls(moduleContext, astTypeInstance)) return false;
			}
			
			return true;
		}
	
		std::vector<SEM::Module*> Run(const std::vector<AST::Module*>& modules) {
			SEM::Namespace * rootNamespace = new SEM::Namespace(Name::Absolute());
			GlobalContext globalContext(rootNamespace);
			
			std::vector<SEM::Module *> semModules;
			for(std::size_t i = 0; i < modules.size(); i++){
				semModules.push_back(new SEM::Module(modules.at(i)->name));
			}
			
			// Build type instances.
			for(std::size_t i = 0; i < modules.size(); i++){
				AST::Module* astModule = modules.at(i);
				if(!AddTypeInstances(globalContext, astModule)){
					return std::vector<SEM::Module*>();
				}
			}
			
			// Fill in type instance member variables.
			for(std::size_t i = 0; i < modules.size(); i++){
				AST::Module* astModule = modules.at(i);
				SEM::Module* semModule = semModules.at(i);
				if(!AddTypeMemberVariables(globalContext, astModule, semModule)){
					return std::vector<SEM::Module*>();
				}
			}
			
			// Build function declarations.
			for(std::size_t i = 0; i < modules.size(); i++){
				AST::Module* astModule = modules.at(i);
				SEM::Module* semModule = semModules.at(i);
				if(!AddFunctionDecls(globalContext, astModule, semModule)){
					return std::vector<SEM::Module*>();
				}
			}
			
			// Fill in function code.
			for(std::size_t i = 0; i < modules.size(); i++){
				AST::Module* astModule = modules.at(i);
				SEM::Module* semModule = semModules.at(i);
				
				if(!ConvertModule(globalContext, astModule, semModule)) {
					return std::vector<SEM::Module*>();
				}
			}
			
			return semModules;
		}
		
	}
	
}

