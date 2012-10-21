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
	
		SEM::TypeInstance::TypeEnum ConvertTypeInstanceEnum(AST::TypeInstance::TypeEnum typeEnum){
			switch(typeEnum){
				case AST::TypeInstance::PRIMITIVE:
					return SEM::TypeInstance::PRIMITIVE;
				case AST::TypeInstance::STRUCT:
					return SEM::TypeInstance::STRUCTDEF;
				case AST::TypeInstance::CLASSDECL:
					return SEM::TypeInstance::CLASSDECL;
				case AST::TypeInstance::CLASSDEF:
					return SEM::TypeInstance::CLASSDEF;
				case AST::TypeInstance::INTERFACE:
					return SEM::TypeInstance::INTERFACE;
				default:
					assert(false && "Unknown type instance type enum");
					return SEM::TypeInstance::CLASSDECL;
			}
		}
	
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
					new SEM::TypeInstance(ConvertTypeInstanceEnum(astTypeInstance->typeEnum),
						namespaceContext.getName() + astTypeInstance->name);
				
				SEM::NamespaceNode node = namespaceContext.getNode(namespaceContext.getName() + astTypeInstance->name);
				if(node.isNamespace()){
					printf("Semantic Analysis Error: type name '%s' clashes with existing namespace name.\n", semTypeInstance->name.toString().c_str());
					return false;
				}else if(node.isTypeInstance()){
					// Types can be unified by name at this point.
					// Later stages will identify whether the types actually match.
					SEM::TypeInstance * semExistingType = node.getTypeInstance();
					assert(semExistingType != NULL);
					if((semExistingType->typeEnum == SEM::TypeInstance::CLASSDECL && semTypeInstance->typeEnum == SEM::TypeInstance::CLASSDEF)
						|| (semExistingType->typeEnum == SEM::TypeInstance::CLASSDEF && semTypeInstance->typeEnum == SEM::TypeInstance::CLASSDECL)){
						// Classes decls and definitions can be unified.
						semExistingType->typeEnum = SEM::TypeInstance::CLASSDEF;
					}else if(semExistingType->typeEnum != semTypeInstance->typeEnum){
						printf("Semantic Analysis Error: non-unifiable types share name '%s'.\n", semTypeInstance->name.toString().c_str());
						return false;
					}
				}else{
					assert(node.isNone() && "Functions shouldn't be added at this point, so anything that isn't a namespace or a type instance should be 'none'");
					const bool addResult = namespaceContext.addTypeInstance(namespaceContext.getName() + astTypeInstance->name, semTypeInstance);
					assert(addResult && "getNode() returned 'none', so adding type instance must succeed");
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
			
			if(!semTypeInstance->variables.empty()){
				assert(astTypeInstance->typeEnum == AST::TypeInstance::STRUCT
					&& "Only structs can be defined with visible member variables twice (and be successfully unified)");
				return true;
			}
			
			if(astTypeInstance->variables.empty()) return true;
			
			assert(astTypeInstance->typeEnum != AST::TypeInstance::PRIMITIVE && astTypeInstance->typeEnum != AST::TypeInstance::INTERFACE
				&& "Primitives and interfaces cannot have member variables");
			
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
			
			Name functionName = context.getName() + astFunction->name;
			
			SEM::NamespaceNode node = context.getNode(functionName);
			
			if(node.isNamespace()){
				printf("Semantic Analysis Error: function name '%s' clashes with existing namespace name.\n", functionName.toString().c_str());
				return false;
			}else if(node.isTypeInstance()){
				printf("Semantic Analysis Error: function name '%s' clashes with existing type name.\n", functionName.toString().c_str());
				return false;
			}else if(node.isFunction()){
				SEM::Function * existingFunction = node.getFunction();
				assert(existingFunction != NULL && "getFunction() must not be NULL as indicated by isFunction() returning true");
				if(!AreTypesEqual(semFunction->type, existingFunction->type)){
					printf("Semantic Analysis Error: declarations of function '%s' don't match.\n", functionName.toString().c_str());
					return false;
				}
				
				// Can unify function declarations.
				return true;
			}else{
				assert(node.isNone() && "Node is not function, type instance, or namespace, so it must be 'none'");
				const bool addResult = context.addFunction(functionName, semFunction);
				assert(addResult && "Adding function must succeed, since name has already been looked up and found to be 'none'");
				return true;
			}
			
			assert(false && "Invalid fallthrough in if stmt of AddFunctionsDecl for function decl");
			return false;
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

