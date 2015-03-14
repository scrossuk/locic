#ifndef LOCIC_SUPPORT_HASHER_HPP
#define LOCIC_SUPPORT_HASHER_HPP

#include <cstddef>

#include <locic/Support/Hash.hpp>

namespace locic{
	
	class Hasher {
	public:
		Hasher();
		
		void addValue(size_t value);
		
		template <typename T>
		void add(const T& object) {
			addValue(hashObject<T>(object));
		}
		
		size_t get() const;
		
	private:
		size_t seed_;
		
	};
	
}

#endif
