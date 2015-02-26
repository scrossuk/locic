#ifndef LOCIC_NAME_HPP
#define LOCIC_NAME_HPP

#include <cassert>
#include <cstddef>
#include <string>

#include <locic/Array.hpp>
#include <locic/String.hpp>

namespace locic{
	
	constexpr size_t NameBaseSize = 4;
	
	class Name{
		typedef Array<String, NameBaseSize> ListType;
		typedef ListType::const_iterator CItType;
		
		public:
			Name();
			
			explicit Name(bool isNameAbsolute);
			
			Name(const Name& name, size_t substrSize);
			
			Name(const Name& prefix, const String& suffix);
			
			Name(const Name& prefix, const Name& suffix);
			
			static Name Absolute();
			
			static Name Relative();
			
			Name(Name&&) = default;
			
			Name copy() const;
			
			Name& operator=(Name&&) = default;
			
			bool operator==(const Name& name) const;
			
			bool operator<(const Name& name) const;
			
			Name operator+(const String& name) const;
			
			Name makeAbsolute(const Name& name) const;
			
			bool isRelative() const;
			
			bool isAbsolute() const;
			
			bool isPrefixOf(const Name& name) const;
			
			bool isExactPrefixOf(const Name& name) const;
			
			std::string toString(bool addPrefix = true) const;
			
			String genString() const;
			
			const String& first() const;
			
			const String& last() const;
			
			const String& onlyElement() const;
			
			bool empty() const;
			
			std::size_t size() const;
			
			const String& at(std::size_t i) const;
			
			const String& revAt(std::size_t i) const;
			
			Name substr(std::size_t substrSize) const;
			
			Name concat(const Name& suffix) const;
			
			Name append(const String& suffix) const;
			
			Name getPrefix() const;
			
			ListType::iterator begin();
			
			ListType::iterator end();
			
			ListType::const_iterator begin() const;
			
			ListType::const_iterator end() const;
			
			std::size_t hash() const;
			
		private:
			Name(const Name&) = delete;
			Name& operator=(const Name&) = delete;
			
			bool isAbsolute_;
			ListType list_;
			
	};

}

namespace std {
	
	template <> struct hash<locic::Name>
	{
		size_t operator()(const locic::Name& nameValue) const
		{
			return nameValue.hash();
		}
	};
	
}

#endif
