#ifndef LOCIC_SUPPORT_UTILS_HPP
#define LOCIC_SUPPORT_UTILS_HPP

namespace locic {
	
	inline bool checkImplies(const bool a, const bool b) {
		return !a || b;
	}
	
	inline bool isPowerOf2(const size_t value) {
		return value != 0 && (value & (value - 1)) == 0;
	}
	
	inline size_t roundUpToAlign(const size_t position, const size_t align) {
		assert(isPowerOf2(align));
		return (position + (align - 1)) & (~(align - 1));
	}
	
}

#endif
