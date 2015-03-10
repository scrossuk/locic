#ifndef LOCIC_SUPPORT_STRING_HPP
#define LOCIC_SUPPORT_STRING_HPP

#include <cassert>
#include <string>

#include <locic/Support/StringHost.hpp>

namespace locic{
	
	class String {
	public:
		String() = default;
		
		String(const StringHost& argHost)
		: host_(&argHost), string_(host_->getCString("")) { }
		
		String(const StringHost& argHost, const char* const cString)
		: host_(&argHost), string_(host_->getCString(cString)) { }
		
		String(const StringHost& argHost, std::string stringValue)
		: host_(&argHost), string_(host_->getString(std::move(stringValue))) { }
		
		String(const StringHost& argHost, const std::string* const stringPointer)
		: host_(&argHost), string_(stringPointer) { }
		
		bool empty() const {
			return string_ != nullptr ? string_->empty() : true;
		}
		
		size_t size() const {
			return string_ != nullptr ? string_->size() : 0;
		}
		
		size_t length() const {
			return size();
		}
		
		size_t capacity() const {
			return string_ != nullptr ? string_->capacity() : 0;
		}
		
		size_t hash() const {
			if (string_ == nullptr) {
				return 0;
			}
			
			std::hash<std::string> hashFn;
			return hashFn(*string_);
		}
		
		char operator[](const size_t index) const {
			assert(index < size());
			return (*string_)[index];
		}
		
		bool starts_with(const std::string& other) const {
			if (string_ == nullptr) {
				return false;
			}
			
			if (other.size() > size()) {
				return false;
			}
			
			for (size_t i = 0; i < other.size(); i++) {
				if ((*this)[i] != other[i]) {
					return false;
				}
			}
			
			return true;
		}
		
		String operator+(const String& other) const {
			assert(string_ != nullptr);
			return String(host(), *string_ + *(other.string_));
		}
		
		String operator+(const std::string& other) const {
			assert(string_ != nullptr);
			return String(host(), *string_ + other);
		}
		
		String substr(const size_t start, const size_t substrLength) const {
			assert(string_ != nullptr);
			return String(host(), string_->substr(start, substrLength));
		}
		
		bool operator==(const String& other) const {
			assert(string_ != nullptr);
			assert(&(host()) == &(other.host()));
			return string_ == other.string_;
		}
		
		bool operator==(const std::string& other) const {
			assert(string_ != nullptr);
			return *string_ == other;
		}
		
		bool operator!=(const std::string& other) const {
			assert(string_ != nullptr);
			return *string_ != other;
		}
		
		bool operator!=(const String& other) const {
			assert(string_ != nullptr);
			assert(&(host()) == &(other.host()));
			return string_ != other.string_;
		}
		
		bool operator<(const String& other) const {
			assert(string_ != nullptr);
			assert(&(host()) == &(other.host()));
			return string_ < other.string_;
		}
		
		const std::string& asStdString() const {
			assert(string_ != nullptr);
			return *string_;
		}
		
		std::string toString() const {
			assert(string_ != nullptr);
			return asStdString();
		}
		
		const char* c_str() const {
			assert(string_ != nullptr);
			return string_->c_str();
		}
		
		const StringHost& host() const {
			assert(string_ != nullptr);
			return *host_;
		}
		
		const std::string* internal() const {
			assert(string_ != nullptr);
			return string_;
		}
		
	private:
		const StringHost* host_;
		const std::string* string_;
		
	};
	
	String CanonicalizeMethodName(const String& name);

}

namespace std {
	
	template <> struct hash<locic::String>
	{
		size_t operator()(const locic::String& stringValue) const
		{
			return stringValue.hash();
		}
	};
	
}

#endif
