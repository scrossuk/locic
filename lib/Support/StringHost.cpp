#include <memory>
#include <string>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/StableSet.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	class StringHostImpl {
	public:
		mutable StableSet<std::string> stringSet;
		mutable FastMap<const char*, const std::string*> cStringMap;
		mutable FastMap<const std::string*, const std::string*> canonicalMap;
		
	};
	
	std::string GetCanonicalString(const std::string& name) {
		std::string canonicalName;
		canonicalName.reserve(name.size());
		
		bool preserveUnderscores = true;
		
		for (size_t i = 0; i < name.size(); i++) {
			const auto c = name[i];
			
			// Preserve underscores at the beginning of the name.
			if (preserveUnderscores && c == '_') {
				canonicalName += '_';
				continue;
			}
			
			preserveUnderscores = false;
			
			if (c == '_') {
				continue;
			}
			
			if (c >= 'A' && c <= 'Z') {
				canonicalName += 'a' + (c - 'A');
				continue;
			}
			
			canonicalName += c;
		}
		
		return canonicalName;
	}
	
	StringHost::StringHost()
	: impl_(new StringHostImpl()) { }
	
	StringHost::~StringHost() { }
	
	const std::string* StringHost::getCString(const char* const cString) const {
		const auto iterator = impl_->cStringMap.find(cString);
		if (iterator != impl_->cStringMap.end()) {
			return iterator->second;
		}
		
		const auto result = getString(cString);
		impl_->cStringMap.insert(std::make_pair(cString, result));
		return result;
	}
	
	const std::string* StringHost::getString(std::string stringValue) const {
		const auto result = impl_->stringSet.insert(std::move(stringValue));
		return &(*(result.first));
	}
	
	const std::string* StringHost::getCanonicalString(const std::string* const stringPointer) const {
		const auto iterator = impl_->canonicalMap.find(stringPointer);
		if (iterator != impl_->canonicalMap.end()) {
			return iterator->second;
		}
		
		const auto canonicalStringPointer = getString(GetCanonicalString(*stringPointer));
		impl_->canonicalMap.insert(std::make_pair(stringPointer, canonicalStringPointer));
		return canonicalStringPointer;
	}
	
}
	
