#ifndef LOCIC_TOKEN_HPP
#define LOCIC_TOKEN_HPP

#include <string>

namespace Locic{

union Token{
	std::string * str;
	int boolValue;
	int intValue;
	float floatValue;
};

}

#endif
