#ifndef LOCIC_SEMANTICANALYSIS_TYPEPROPERTIES_HPP
#define LOCIC_SEMANTICANALYSIS_TYPEPROPERTIES_HPP

#include <string>
#include <vector>

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Value* CallPropertyFunction(SEM::Type* type, const std::string& propertyName, const std::vector<SEM::Value*>& args);
		
		SEM::Value* CallPropertyMethod(SEM::Value* value, const std::string& propertyName, const std::vector<SEM::Value*>& args);
		
	}
	
}

#endif
