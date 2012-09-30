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
		bool AddTypeInstances(Context& context, AST::Namespace* astNamespace){
			SEM::Namespace* semNamespace = context.getNode(context.getName() + astNamespace->name).getNamespace();
			assert(semNamespace != NULL);
			
			NamespaceContext namespaceContext(context, semNamespace);
		
			//-- Get all namespaces.
			for(std::size_t i = 0; i < astNamespace->namespaces.size(); i++){
				AST::Namespace* astChildNamespace = astNamespace->namespaces.at(i);
				SEM::Namespace * semChildNamespace =
					new SEM::Namespace(namespaceContext.getName() + astChildNamespace->name);
				
				if(!namespaceContext.addNamespace(semChildNamespace->name, semChildNamespace)){
					printf("Semantic Analysis Error: namespace already defined with name '%s'.\n", semChildNamespace->name.toString().c_str());
					return false;
				}
				
				if(!AddTypeInstances(namespaceContext, astChildNamespace)) return false;
			}
			
			//-- Get all type names.
			for(std::size_t i = 0; i < astNamespace->typeInstances.size(); i++){
				AST::TypeInstance* astTypeInstance = astNamespace->typeInstances.at(i);
				SEM::TypeInstance * semTypeInstance =
					new SEM::TypeInstance((SEM::TypeInstance::TypeEnum) astTypeInstance->typeEnum,
						namespaceContext.getName() + astTypeInstance->name);
				
				if(!namespaceContext.addTypeInstance(namespaceContext.getName() + astTypeInstance->name, semTypeInstance)){
					printf("Semantic Analysis Error: type already defined with name '%s'.\n", semTypeInstance->name.toString().c_str());
					return false;
				}
			}
			return true;
		}
		
		bool AddTypeInstances(Context& context, AST::Module* astModule, SEM::Module * semModule){
			ModuleContext moduleContext(context, semModule);
			
			return AddTypeInstances(moduleContext, astModule->nameSpace);
		}
		
		// Fill in type instance structures with member variable information.
		bool AddTypeMemberVariables(Context& context, AST::TypeInstance * astTypeInstance){
			SEM::TypeInstance* semTypeInstance = context.getNode(context.getName() + astTypeInstance->name).getTypeInstance();
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
		
		bool AddTypeMemberVariables(Context& context, AST::Namespace * astNamespace){
			SEM::Namespace * semNamespace = context.getNode(context.getName() + astNamespace->name).getNamespace();
			assert(semNamespace != NULL);
		
			NamespaceContext namespaceContext(context, semNamespace);
			
			for(std::size_t i = 0; i < astNamespace->namespaces.size(); i++){
				AST::Namespace* astChildNamespace = astNamespace->namespaces.at(i);
				if(!AddTypeMemberVariables(namespaceContext, astChildNamespace)) return false;
			}
			
			for(std::size_t i = 0; i < astNamespace->typeInstances.size(); i++){
				AST::TypeInstance* astTypeInstance = astNamespace->typeInstances.at(i);
				if(!AddTypeMemberVariables(namespaceContext, astTypeInstance)) return false;
			}
			
			return true;
		}
		
		bool AddTypeMemberVariables(Context& context, AST::Module * astModule, SEM::Module * semModule){
			ModuleContext moduleContext(context, semModule);
			
			return AddTypeMemberVariables(moduleContext, astModule->nameSpace);
		}
		
		bool AddFunctionDecls(Context& context, AST::Function* astFunction){
			SEM::Function * semFunction = ConvertFunctionDecl(context, astFunction);
			if(semFunction == NULL) return false;
			
			SEM::Function * existingFunction = context.getNode(context.getName() + astFunction->name).getFunction();
			if(existingFunction != NULL){
				if(!AreTypesEqual(semFunction->type, existingFunction->type)){
					printf("Semantic Analysis Error: declarations of function '%s' don't match.\n", astFunction->getFullName().c_str());
					return false;
				}
				return true;
			}
				
			if(!context.addFunction(context.getName() + astFunction->name, semFunction, astFunction->isMethod)) {
				printf("Semantic Analysis Error: function name '%s' clashes with existing type name.\n", astFunction->getFullName().c_str());
				return false;
			}
			
			return true;
		}
		
		bool AddFunctionDecls(Context& context, AST::TypeInstance* astTypeInstance){
			SEM::TypeInstance* semTypeInstance = context.getNode(context.getName() + astTypeInstance->name).getTypeInstance();
			assert(semTypeInstance != NULL);
			
			TypeInstanceContext typeInstanceContext(context, semTypeInstance);
			
			for(std::size_t i = 0; i < astTypeInstance->functions.size(); i++){
				AST::Function * astFunction = astTypeInstance->functions.at(i);
				if(!AddFunctionDecls(typeInstanceContext, astFunction)) return false;
			}
			
			return true;
		}
		
		bool AddFunctionDecls(Context& context, AST::Namespace * astNamespace){
			SEM::Namespace * semNamespace = context.getNode(context.getName() + astNamespace->name).getNamespace();
			assert(semNamespace != NULL);
			
			NamespaceContext namespaceContext(context, semNamespace);
		
			for(std::size_t i = 0; i < astNamespace->functions.size(); i++){
				AST::Function * astFunction = astNamespace->functions.at(i);
				if(!AddFunctionDecls(namespaceContext, astFunction)) return false;
			}
			
			for(std::size_t i = 0; i < astNamespace->namespaces.size(); i++){
				AST::Namespace * astChildNamespace = astNamespace->namespaces.at(i);
				if(!AddFunctionDecls(namespaceContext, astChildNamespace)) return false;
			}
			
			for(std::size_t i = 0; i < astNamespace->typeInstances.size(); i++){
				AST::TypeInstance * astTypeInstance = astNamespace->typeInstances.at(i);
				if(!AddFunctionDecls(namespaceContext, astTypeInstance)) return false;
			}
			
			return true;
		}
		
		bool AddFunctionDecls(Context& context, AST::Module* astModule, SEM::Module* semModule){
			ModuleContext moduleContext(context, semModule);
			return AddFunctionDecls(moduleContext, astModule->nameSpace);
		}
	
		std::vector<SEM::Module*> Run(const std::vector<AST::Module*>& modules) {
			SEM::Namespace * rootNamespace = new SEM::Namespace(Name::Absolute());
			RootContext rootContext(rootNamespace);
			
			std::vector<SEM::Module *> semModules;
			for(std::size_t i = 0; i < modules.size(); i++){
				semModules.push_back(new SEM::Module(modules.at(i)->name));
			}
			
			// Build type instances.
			for(std::size_t i = 0; i < modules.size(); i++){
				AST::Module* astModule = modules.at(i);
				SEM::Module* semModule = semModules.at(i);
				if(!AddTypeInstances(rootContext, astModule, semModule)){
					return std::vector<SEM::Module*>();
				}
			}
			
			// Fill in type instance member variables.
			for(std::size_t i = 0; i < modules.size(); i++){
				AST::Module* astModule = modules.at(i);
				SEM::Module* semModule = semModules.at(i);
				if(!AddTypeMemberVariables(rootContext, astModule, semModule)){
					return std::vector<SEM::Module*>();
				}
			}
			
			// Build function declarations.
			for(std::size_t i = 0; i < modules.size(); i++){
				AST::Module* astModule = modules.at(i);
				SEM::Module* semModule = semModules.at(i);
				if(!AddFunctionDecls(rootContext, astModule, semModule)){
					return std::vector<SEM::Module*>();
				}
			}
			
			// Fill in function code.
			for(std::size_t i = 0; i < modules.size(); i++){
				AST::Module* astModule = modules.at(i);
				SEM::Module* semModule = semModules.at(i);
				
				if(!ConvertModule(rootContext, astModule, semModule)) {
					return std::vector<SEM::Module*>();
				}
			}
			
			return semModules;
		}
		
	}
	
}

