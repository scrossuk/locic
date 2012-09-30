#include <cassert>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool ConvertClassDef(Context& context, AST::TypeInstance* typeInstance) {
			SEM::TypeInstance * semTypeInstance = context.getNode(context.getName() + typeInstance->name).getTypeInstance();
			
			assert(semTypeInstance != NULL);
			assert(semTypeInstance->typeEnum == SEM::TypeInstance::CLASSDEF);
			
			TypeInstanceContext typeInstanceContext(context, semTypeInstance);
			
			for(std::size_t i = 0; i < typeInstance->functions.size(); i++){
				if(!ConvertFunctionDef(typeInstanceContext, typeInstance->functions.at(i))) return false;
			}
			
			return true;
		}
		
	}
	
}

