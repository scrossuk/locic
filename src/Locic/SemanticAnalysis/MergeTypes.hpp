#ifndef LOCIC_SEMANTICANALYSIS_MERGETYPES_HPP
#define LOCIC_SEMANTICANALYSIS_MERGETYPES_HPP

#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::TypeInstance* MergeClasses(SEM::TypeInstance* first, SEM::TypeInstance* second);
		
		SEM::TypeInstance* MergeStructs(SEM::TypeInstance* first, SEM::TypeInstance* second);
		
		SEM::TypeInstance* MergeTypeInstances(SEM::TypeInstance* first, SEM::TypeInstance* second);
		
		SEM::TypeInstance::Kind MergeTypeInstanceKinds(const Name& fullTypeName, SEM::TypeInstance::Kind first, SEM::TypeInstance::Kind second);
		
	}
	
}

#endif
