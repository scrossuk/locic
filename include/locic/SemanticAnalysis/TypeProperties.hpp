#ifndef LOCIC_SEMANTICANALYSIS_TYPEPROPERTIES_HPP
#define LOCIC_SEMANTICANALYSIS_TYPEPROPERTIES_HPP

#include <string>
#include <vector>

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Value* GetStaticMethod(SEM::Type* type, const std::string& methodName);
		
		SEM::Value* GetMethod(SEM::Value* value, const std::string& methodName);
		
		SEM::Value* CallValue(SEM::Value* value, const std::vector<SEM::Value*>& args, const Debug::SourceLocation& location);
		
		bool supportsImplicitCopy(SEM::Type* type);
		
	}
	
}

#endif
