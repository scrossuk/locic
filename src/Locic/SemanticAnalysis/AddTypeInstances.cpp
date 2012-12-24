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
		
			//-- Initial phase: get all type names.
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
		
	}
	
}

