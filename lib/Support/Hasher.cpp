#include <cstddef>

#include <locic/Support/Hasher.hpp>

namespace locic{
	
	Hasher::Hasher()
	: seed_(0) { }
	
	void Hasher::addValue(const size_t value) {
		seed_ ^= value + 0x9e3779b9 + (seed_ << 6) + (seed_ >> 2);
	}
	
	size_t Hasher::get() const {
		return seed_;
	}
	
}
