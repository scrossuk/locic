#ifndef LOCIC_PARSER_TOKEN_HPP
#define LOCIC_PARSER_TOKEN_HPP

#include <string>

namespace Locic{

	namespace Parser{
		
		union Token{
			std::string * str;
			bool boolValue;
			int intValue;
			float floatValue;
		};
		
	}

}

#endif
