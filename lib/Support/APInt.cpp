#include <cstddef>
#include <sstream>

#include <boost/multiprecision/cpp_int.hpp>

#include <locic/Support/APInt.hpp>

namespace locic{
	
	class APIntImpl {
	public:
		typedef boost::multiprecision::cpp_int ValueType;
		
		APIntImpl(const uint64_t argValue)
		: value_(argValue) { }
		
		APIntImpl(const APIntImpl&) = default;
		
		ValueType& value() {
			return value_;
		}
		
		const ValueType& value() const {
			return value_;
		}
		
	private:
		ValueType value_;
		
	};
	
	APInt::APInt()
	: impl_(nullptr) { }
	
	APInt::APInt(const uint64_t value)
	: impl_(new APIntImpl(value)) { }
	
	APInt::APInt(const APInt& other)
	: impl_(new APIntImpl(*(other.impl_))) { }
	
	APInt::APInt(APInt&& other)
	: impl_(std::move(other.impl_)) { }
	
	APInt& APInt::operator=(APInt&& other) {
		impl_ = std::move(other.impl_);
		return *this;
	}
	
	APInt::~APInt() { }
	
	APInt APInt::copy() const {
		return APInt(*this);
	}
	
	APInt& APInt::operator+=(const APInt& other) {
		impl_->value() += other.impl_->value();
		return *this;
	}
	
	APInt& APInt::operator*=(const APInt& other) {
		impl_->value() *= other.impl_->value();
		return *this;
	}
	
	bool APInt::operator==(const APInt& other) const {
		return impl_->value() == other.impl_->value();
	}
	
	bool APInt::operator!=(const APInt& other) const {
		return impl_->value() != other.impl_->value();
	}
	
	bool APInt::operator<(const APInt& other) const {
		return impl_->value() < other.impl_->value();
	}
	
	bool APInt::operator<=(const APInt& other) const {
		return impl_->value() <= other.impl_->value();
	}
	
	bool APInt::operator>(const APInt& other) const {
		return impl_->value() > other.impl_->value();
	}
	
	bool APInt::operator>=(const APInt& other) const {
		return impl_->value() >= other.impl_->value();
	}
	
	uint64_t APInt::asUint64() const {
		return static_cast<uint64_t>(impl_->value());
	}
	
	size_t APInt::hash() const {
		return static_cast<size_t>(impl_->value());
	}
	
	std::string APInt::toString() const {
		std::ostringstream stream;
		stream << impl_->value();
		return stream.str();
	}
	
}
