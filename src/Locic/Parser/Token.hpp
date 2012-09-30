#ifndef LOCIC_PARSER_TOKEN_HPP
#define LOCIC_PARSER_TOKEN_HPP

#include <string>

namespace Locic{
	
	namespace Parser{
		
		// Resolve circular reference.
		struct Context;
		
	}
	
}

#include <Locic/Parser/GeneratedParser.hpp>

namespace Locic{

	namespace Parser{
		
		typedef LOCIC_PARSER_GENERATEDPARSER_STYPE Token;
		
	}

}

#endif
