#ifndef LOCIC_SUPPORT_HASH_HPP
#define LOCIC_SUPPORT_HASH_HPP

#include <cstddef>
#include <functional>
#include <type_traits>

namespace locic{
	
	/**
	 * \brief Detect if object supports hash() method.
	 */
	template <typename T>
	class hasHashMethod
	{
		typedef char one;
		typedef long two;
		
		template <typename C> static one test( decltype(&C::hash) ) ;
		template <typename C> static two test(...);
		
	public:
		enum { value = sizeof(test<T>(0)) == sizeof(char) };
	};
	
	/**
	 * \brief Hash an object.
	 */
	template <typename T>
	typename std::enable_if<hasHashMethod<T>::value, size_t>::type
	hashObject(const T& object) {
		return object.hash();
	}
	
	template <typename T>
	typename std::enable_if<!hasHashMethod<T>::value && std::is_enum<T>::value, size_t>::type
	hashObject(const T& object) {
		return std::hash<typename std::underlying_type<T>::type>()(object);
	}
	
	template <typename T>
	typename std::enable_if<!hasHashMethod<T>::value && !std::is_enum<T>::value, size_t>::type
	hashObject(const T& object) {
		return std::hash<T>()(object);
	}
	
}

#endif
