#ifndef LOCIC_STRING_HPP
#define LOCIC_STRING_HPP

#include <string>

namespace Locic{

	std::string makeString(const char * format, ...)
		__attribute__((format(printf, 1, 2)));

}

#endif
