#include <cassert>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::TypeInstance * ConvertClassDef(StructuralContext& context, AST::TypeInstance* typeInstance) {
			const std::string typeName = typeInstance->name;
			
			SEM::TypeInstance * semTypeInstance = context.getTypeInstance(context.getName() + typeName);
			
			assert(semTypeInstance != NULL);
			assert(semTypeInstance->typeEnum == SEM::TypeInstance::CLASSDEF);
			
			TypeInstanceContext typeInstanceContext(context, semTypeInstance);
			
			for(std::size_t i = 0; i < typeInstance->constructors.size(); i++){
				SEM::Function * function = ConvertFunctionDef(typeInstanceContext, typeInstance->constructors.at(i));
				if(function == NULL) return NULL;
			}
			
			for(std::size_t i = 0; i < typeInstance->functions.size(); i++){
				SEM::Function * function = ConvertFunctionDef(typeInstanceContext, typeInstance->functions.at(i), true);
				if(function == NULL) return NULL;
			}
			
			return semTypeInstance;
		}
		
	}
	
}

