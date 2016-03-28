#ifndef LOCIC_SUPPORT_APINT_HPP
#define LOCIC_SUPPORT_APINT_HPP

#include <cstdint>
#include <memory>
#include <string>

namespace locic {
	
	class APIntImpl;
	
	class APInt {
	public:
		APInt();
		APInt(uint64_t value);
		~APInt();
		
		APInt(APInt&& other);
		APInt& operator=(APInt&& other);
		
		APInt copy() const;
		
		APInt& operator+=(const APInt& other);
		
		APInt& operator*=(const APInt& other);
		
		bool operator==(const APInt& other) const;
		bool operator!=(const APInt& other) const;
		bool operator<(const APInt& other) const;
		bool operator<=(const APInt& other) const;
		bool operator>(const APInt& other) const;
		bool operator>=(const APInt& other) const;
		
		uint64_t asUint64() const;
		
		size_t hash() const;
		
		std::string toString() const;
		
	private:
		APInt(const APInt& other);
		
		APInt& operator=(const APInt& other) = delete;
		
		std::unique_ptr<APIntImpl> impl_;
		
	};
	
}

#endif
