#ifndef LOCIC_SEMANTICANALYSIS_DEFAULTMETHODS_HPP
#define LOCIC_SEMANTICANALYSIS_DEFAULTMETHODS_HPP

#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Function* CreateDefaultConstructor(SEM::TypeInstance* typeInstance);
		
		bool HasDefaultImplicitCopy(SEM::TypeInstance* typeInstance);
		
		SEM::Function* CreateDefaultImplicitCopy(SEM::TypeInstance* typeInstance);
		
		SEM::Function* CreateDefaultCompare(Context& context, SEM::TypeInstance* typeInstance);
		
		SEM::Function* CreateDefaultMethod(Context& context, SEM::TypeInstance* typeInstance, bool isStatic, const Name& name);
		
	}
	
}

#endif
