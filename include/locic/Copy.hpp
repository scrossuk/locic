#ifndef LOCIC_COPY_HPP
#define LOCIC_COPY_HPP

#include <type_traits>

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
	
}

#endif
