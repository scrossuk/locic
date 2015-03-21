#ifndef LOCIC_MAKESTRING_HPP
#define LOCIC_MAKESTRING_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace locic{

	std::string makeString(const char * format, ...)
		__attribute__((format(printf, 1, 2)));
	
	template <typename T>
	inline std::string typeToString(const T& value) {
		return value.toString();
	}
	
	template <typename T>
	inline std::string typeToString(T* const value) {
		return value->toString();
	}
	
	template <typename T>
	inline std::string typeToString(const std::unique_ptr<T>& value) {
		return value->toString();
	}
	
	inline std::string typeToString(const std::string& value) {
		return value;
	}
	
	template <typename T>
	std::string makeArrayString(const T& array){
		auto s = makeString("Array [size = %llu] {",
			(unsigned long long) array.size());
		
		for(size_t i = 0; i < array.size(); i++){
			if(i > 0) s += ", ";
			s += makeString("%llu: %s",
				(unsigned long long) i,
				array.at(i).toString().c_str());
		}
		
		s += "}";
		
		return s;
	}
	
	template <typename T>
	std::string makeArrayPtrString(const T& array){
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
	std::string makeMapString(const T& map){
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
				typeToString(pair.first).c_str(),
				typeToString(pair.second).c_str());
		}
		
		s += "}";
		
		return s;
	}
	
	template <typename T>
	std::string makeNameArrayString(const T& array){
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
