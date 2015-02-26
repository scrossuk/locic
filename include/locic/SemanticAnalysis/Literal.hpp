#ifndef LOCIC_SEMANTICANALYSIS_LITERAL_HPP
#define LOCIC_SEMANTICANALYSIS_LITERAL_HPP

#include <string>

#include <locic/Constant.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Value getLiteralValue(Context& context, const String& specifier, const Constant& constant, const Debug::SourceLocation& location);
		
	}
	
}

#endif
