#include <cstddef>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertClassDef.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <Locic/SemanticAnalysis/ConvertNamespace.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		void ConvertNamespace(Context& context, AST::Namespace* astNamespace, SEM::Namespace * semNamespace){
			NamespaceContext namespaceContext(context, semNamespace);
			
			for(std::size_t i = 0; i < astNamespace->functions.size(); i++) {
				AST::Function* astFunction = astNamespace->functions.at(i);
				
				if(astFunction->typeEnum == AST::Function::DEFINITION){
					SEM::Function * semFunction = namespaceContext.getNode(namespaceContext.getName() + astFunction->name).getFunction();
					assert(semFunction != NULL);
					ConvertFunctionDef(namespaceContext, astFunction, semFunction);
				}
			}
			
			for(std::size_t i = 0; i < astNamespace->namespaces.size(); i++) {
				AST::Namespace* astChildNamespace = astNamespace->namespaces.at(i);
				SEM::Namespace * semChildNamespace = namespaceContext.getNode(namespaceContext.getName() + astChildNamespace->name).getNamespace();
				assert(semChildNamespace != NULL);
				ConvertNamespace(namespaceContext, astChildNamespace, semChildNamespace);
			}
			
			for(std::size_t i = 0; i < astNamespace->typeInstances.size(); i++){
				AST::TypeInstance * astTypeInstance = astNamespace->typeInstances.at(i);
				
				if(astTypeInstance->typeEnum == AST::TypeInstance::CLASSDEF){
					SEM::TypeInstance * semTypeInstance = namespaceContext.getNode(namespaceContext.getName() + astTypeInstance->name).getTypeInstance();
					assert(semTypeInstance != NULL);
					ConvertClassDef(namespaceContext, astTypeInstance, semTypeInstance);
				}
			}
		}
		
	}
	
}

