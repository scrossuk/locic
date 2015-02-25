#ifndef LOCIC_NAME_HPP
#define LOCIC_NAME_HPP

#include <cassert>
#include <cstddef>
#include <string>

#include <locic/Array.hpp>

namespace locic{
	
	constexpr size_t NameBaseSize = 4;
	
	class Name{
		typedef Array<std::string, NameBaseSize> ListType;
		typedef ListType::const_iterator CItType;
		
		public:
			Name();
			
			explicit Name(bool isNameAbsolute);
			
			Name(const Name& name, size_t substrSize);
			
			Name(const Name& prefix, const std::string& suffix);
			
			Name(const Name& prefix, const Name& suffix);
			
			static Name Absolute();
			
			static Name Relative();
			
			Name(Name&&) = default;
			
			Name copy() const;
			
			Name& operator=(Name&&) = default;
			
			bool operator==(const Name& name) const;
			
			Name operator+(const std::string& name) const;
			
			Name makeAbsolute(const Name& name) const;
			
			bool isRelative() const;
			
			bool isAbsolute() const;
			
			bool isPrefixOf(const Name& name) const;
			
			bool isExactPrefixOf(const Name& name) const;
			
			std::string toString(bool addPrefix = true) const;
			
			std::string genString() const;
			
			const std::string& first() const;
			
			const std::string& last() const;
			
			const std::string& onlyElement() const;
			
			bool empty() const;
			
			std::size_t size() const;
			
			const std::string& at(std::size_t i) const;
			
			const std::string& revAt(std::size_t i) const;
			
			Name substr(std::size_t substrSize) const;
			
			Name concat(const Name& suffix) const;
			
			Name append(const std::string& suffix) const;
			
			Name getPrefix() const;
			
			ListType::iterator begin();
			
			ListType::iterator end();
			
			ListType::const_iterator begin() const;
			
			ListType::const_iterator end() const;
			
		private:
			Name(const Name&) = delete;
			Name& operator=(const Name&) = delete;
			
			bool isAbsolute_;
			ListType list_;
			
	};
	
	std::string CanonicalizeMethodName(const std::string& name);

}

#endif
