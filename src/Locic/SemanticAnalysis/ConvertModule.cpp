#include <cstddef>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertClassDef.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <Locic/SemanticAnalysis/ConvertModule.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool ConvertNamespace(Context& context, AST::Namespace* nameSpace){
			SEM::Namespace * semNamespace = context.getNode(context.getName() + nameSpace->name).getNamespace();
			assert(semNamespace != NULL);
			
			NamespaceContext namespaceContext(context, semNamespace);
			
			for(std::size_t i = 0; i < nameSpace->functions.size(); i++) {
				AST::Function* astFunction = nameSpace->functions.at(i);
				
				if(astFunction->typeEnum == AST::Function::DEFINITION){
					if(!ConvertFunctionDef(namespaceContext, astFunction)) return false;
				}
			}
			
			for(std::size_t i = 0; i < nameSpace->typeInstances.size(); i++){
				AST::TypeInstance * astTypeInstance = nameSpace->typeInstances.at(i);
				
				if(astTypeInstance->typeEnum == AST::TypeInstance::CLASSDEF){
					if(!ConvertClassDef(namespaceContext, astTypeInstance)) return false;
				}
			}
			
			return true;
		}
	
		bool ConvertModule(Context& context, AST::Module* module, SEM::Module* semModule) {
			ModuleContext moduleContext(context, semModule);
			
			return ConvertNamespace(moduleContext, module->nameSpace);
		}
		
	}
	
}

