#ifndef LOCIC_MAKEARRAY_HPP
#define LOCIC_MAKEARRAY_HPP

#include <vector>

namespace locic{

	template<typename T>
	void appendToArray(std::vector<T>&) { }
	
	template<typename T, typename... Args>
	void appendToArray(std::vector<T>& array, T arg, Args... args) {
		array.push_back(std::move(arg));
		appendToArray(array, std::move(args)...);
	}

	template<typename T, typename... Args>
	std::vector<T> makeArray(T arg, Args... args) {
		std::vector<T> array;
		array.reserve(1 + sizeof...(Args));
		appendToArray(array, std::move(arg), std::move(args)...);
		return array;
	}

}

#endif
