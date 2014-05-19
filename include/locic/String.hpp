#ifndef LOCIC_STRING_HPP
#define LOCIC_STRING_HPP

#include <map>
#include <string>
#include <vector>

namespace locic{

	std::string makeString(const char * format, ...)
		__attribute__((format(printf, 1, 2)));
	
	template <typename T>
	std::string makeArrayString(const std::vector<T>& array){
		auto s = makeString("Array [size = %llu] {",
			(unsigned long long) array.size());
		
		for(size_t i = 0; i < array.size(); i++){
			if(i > 0) s += ", ";
			s += makeString("%llu: %s",
				(unsigned long long) i,
				array.at(i)->toString().c_str());
		}
		
		s += "}";
		
		return s;
	}
	
	template <typename T>
	std::string makeMapString(const std::map<std::string, T>& map){
		auto s = makeString("Map [size = %llu] {",
			(unsigned long long) map.size());
		
		bool isFirst = true;
		for (const auto& pair: map) {
			if (isFirst) {
				isFirst = false;
			} else {
				s += ", ";
			}
			
			s += makeString("%s: %s",
				pair.first.c_str(),
				pair.second->toString().c_str());
		}
		
		s += "}";
		
		return s;
	}
	
	template <typename T>
	std::string makeNameArrayString(const std::vector<T>& array){
		std::string s = makeString("Array[size = %llu]{",
			(unsigned long long) array.size());;
		
		for(size_t i = 0; i < array.size(); i++){
			if(i > 0) s += ", ";
			s += makeString("%llu: %s",
				(unsigned long long) i,
				array.at(i)->nameToString().c_str());
		}
		
		s += "}";
		
		return s;
	}
	
	std::string escapeString(const std::string& string);
	
	std::string formatMessage(const std::string& message);
	
	std::vector<std::string> splitString(const std::string& str, const std::string& separator);

}

#endif
