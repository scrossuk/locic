#ifndef LOCIC_SUPPORT_COPY_HPP
#define LOCIC_SUPPORT_COPY_HPP

#include <type_traits>
#include <utility>

namespace locic{
	
	/**
	 * \brief Copy an object.
	 * 
	 * Loci has various types that use an explicit
	 * copy() method as a clearer alternative to
	 * copy constructors; this function calls that
	 * method if available or a copy constructor
	 * otherwise.
	 */
	template <typename T>
	typename std::enable_if<std::is_copy_constructible<T>::value, T>::type
	copyObject(const T& object) {
		return T(object);
	}
	
	template <typename T>
	typename std::enable_if<!std::is_copy_constructible<T>::value, T>::type
	copyObject(const T& object) {
		return object.copy();
	}
	
	template <typename A, typename B>
	std::pair<A, B> copyObject(const std::pair<A, B>& object) {
		return std::make_pair(copyObject(object.first), copyObject(object.second));
	}
	
}

#endif
