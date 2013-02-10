#ifndef LOCIC_STRING_HPP
#define LOCIC_STRING_HPP

#include <string>
#include <vector>

namespace Locic{

	std::string makeString(const char * format, ...)
		__attribute__((format(printf, 1, 2)));
	
	template <typename T>
	std::string makeArrayString(const std::vector<T>& array){
		std::string s = makeString("Array[size = %llu]{",
			(unsigned long long) array.size());;
		
		for(size_t i = 0; i < array.size(); i++){
			if(i > 0) s += ", ";
			s += makeString("%llu: %s",
				(unsigned long long) i,
				array.at(i)->toString().c_str());
		}
		
		s += "}";
		
		return s;
	}
	
	std::string escapeString(const std::string& string);

}

#endif
