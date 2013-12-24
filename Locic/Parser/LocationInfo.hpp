#ifndef LOCIC_PARSER_LOCATIONINFO_HPP
#define LOCIC_PARSER_LOCATIONINFO_HPP

#include <string>

namespace Locic{
	
	namespace Parser{
		
		// Resolve circular reference.
		class Context;
		
	}
	
}

#include <Locic/Parser/GeneratedParser.hpp>

namespace Locic{

	namespace Parser{
		
		typedef LOCIC_PARSER_GENERATEDPARSER_LTYPE LocationInfo;
		
	}

}

#endif
