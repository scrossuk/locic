#include <cassert>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::TypeInstance * ConvertClassDef(Context& context, AST::TypeInstance* typeInstance) {
			const std::string typeName = typeInstance->getFullName();
			
			SEM::TypeInstance * semTypeInstance = context.getTypeInstance(typeName);
			
			assert(semTypeInstance != NULL);
			assert(semTypeInstance->typeEnum == SEM::TypeInstance::CLASSDEF);
			
			TypeInstanceContext typeInstanceContext(context, semTypeInstance);
			
			for(std::size_t i = 0; i < typeInstance->functions.size(); i++){
				SEM::Function * function = ConvertFunctionDef(typeInstanceContext, typeInstance->functions.at(i));
				
				if(function == NULL) return NULL;
				
				semTypeInstance->methods.push_back(function);
			}
			
			return semTypeInstance;
		}
		
	}
	
}

