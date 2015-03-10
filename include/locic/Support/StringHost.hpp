#ifndef LOCIC_SUPPORT_STRINGHOST_HPP
#define LOCIC_SUPPORT_STRINGHOST_HPP

#include <memory>
#include <string>

namespace locic{
	
	class StringHost {
	public:
		StringHost();
		~StringHost();
		
		const std::string* getCString(const char* const cString) const;
		
		const std::string* getString(std::string stringValue) const;
		
		const std::string* getCanonicalString(const std::string* stringPtr) const;
		
	private:
		StringHost(const StringHost&) = delete;
		StringHost& operator=(const StringHost&) = delete;
		
		std::unique_ptr<class StringHostImpl> impl_;
		
	};
	
}

#endif
