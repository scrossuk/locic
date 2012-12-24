#include <cassert>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		void ConvertClassDef(Context& context, AST::TypeInstance* astTypeInstance, SEM::TypeInstance* semTypeInstance) {
			assert(semTypeInstance->typeEnum == SEM::TypeInstance::CLASSDEF);
			
			TypeInstanceContext typeInstanceContext(context, semTypeInstance);
			
			for(std::size_t i = 0; i < astTypeInstance->functions.size(); i++){
				AST::Function* astFunction = astTypeInstance->functions.at(i);
				SEM::Function * semFunction = typeInstanceContext.getNode(typeInstanceContext.getName() + astFunction->name).getFunction();
				assert(semFunction != NULL && "Convert function definition requires an already-converted corresponding function declaration");
				ConvertFunctionDef(typeInstanceContext, astFunction, semFunction);
			}
		}
		
	}
	
}

