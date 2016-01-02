#ifndef LOCIC_PARSER_GENERATEDTOKEN_HPP
#define LOCIC_PARSER_GENERATEDTOKEN_HPP

#include <string>

namespace locic{
	
	namespace Parser{
		
		// Resolve circular reference.
		class Context;
		
	}
	
}

#include "GeneratedParser.hpp"

namespace locic{

	namespace Parser{
		
		typedef LOCIC_PARSER_GENERATEDPARSER_STYPE GeneratedToken;
		
	}

}

#endif
