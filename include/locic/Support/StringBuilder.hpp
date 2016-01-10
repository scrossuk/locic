#ifndef LOCIC_SUPPORT_STRINGBUILDER_HPP
#define LOCIC_SUPPORT_STRINGBUILDER_HPP

#include <cassert>
#include <string>

#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	class StringBuilder {
	public:
		StringBuilder(const StringHost& argHost)
		: host_(argHost) { }
		
		bool empty() const {
			return string_.empty();
		}
		
		size_t size() const {
			return string_.size();
		}
		
		size_t length() const {
			return string_.size();
		}
		
		size_t capacity() const {
			return string_.capacity();
		}
		
		void reserve(const size_t newCapacity) {
			string_.reserve(newCapacity);
		}
		
		void append(const char c) {
			string_ += c;
		}
		
		String getString() const {
			return String(host_, string_);
		}
		
	private:
		const StringHost& host_;
		std::string string_;
		
	};

}

#endif
