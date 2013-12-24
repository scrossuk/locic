#ifndef LOCIC_SEMANTICANALYSIS_TYPEPROPERTIES_HPP
#define LOCIC_SEMANTICANALYSIS_TYPEPROPERTIES_HPP



namespace Locic {

	namespace SemanticAnalysis {
		
		SEM::Value* CallProperty(SEM::Value* value, const std::string& propertyName, const std::vector<SEM::Value*>& args);
		
	}
	
}

#endif
