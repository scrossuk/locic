#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

#include <locic/Name.hpp>

namespace locic {

	Name::Name()
	: isAbsolute_(true){ }
	
	Name::Name(bool isNameAbsolute)
	: isAbsolute_(isNameAbsolute){ }
	
	Name::Name(const Name& name, size_t substrSize)
	: isAbsolute_(name.isAbsolute()){
		
		assert(substrSize <= name.size());
		
		for(std::size_t i = 0; i < substrSize; i++){
			list_.push_back(name.at(i));
		}	
	}
	
	Name::Name(const Name& prefix, const std::string& suffix)
	: isAbsolute_(prefix.isAbsolute()){
		for(std::size_t i = 0; i < prefix.size(); i++){
			list_.push_back(prefix.at(i));
		}
		
		// No member of a name can be an empty string.
		if(suffix.size() > 0){
			list_.push_back(suffix);
		}
	}
	
	Name::Name(const Name& prefix, const Name& suffix)
	: isAbsolute_(prefix.isAbsolute()){
		for(std::size_t i = 0; i < prefix.size(); i++){
			list_.push_back(prefix.at(i));
		}
		for(std::size_t i = 0; i < suffix.size(); i++){
			list_.push_back(suffix.at(i));
		}
	}
	
	Name Name::Absolute(){
		return Name(true);
	}
	
	Name Name::Relative(){
		return Name(false);
	}
	
	bool Name::operator==(const Name& name) const{
		if(size() != name.size()) return false;
		for(std::size_t i = 0; i < size(); i++){
			if(at(i) != name.at(i)){
				return false;
			}
		}
		return true;
	}
	
	Name Name::operator+(const std::string& name) const{
		return Name(*this, name);
	}
	
	Name Name::makeAbsolute(const Name& name) const{
		assert(isAbsolute());
		assert(!name.empty());
		if(name.isAbsolute()){
			return name;
		}else{
			return concat(name);
		}
	}
	
	bool Name::isRelative() const{
		return !isAbsolute_;
	}
	
	bool Name::isAbsolute() const{
		return isAbsolute_;
	}
	
	bool Name::isPrefixOf(const Name& name) const{
		if(size() >= name.size()) return false;
		for(std::size_t i = 0; i < size(); i++){
			if(at(i) != name.at(i)){
				return false;
			}
		}
		return true;
	}
	
	bool Name::isExactPrefixOf(const Name& name) const{
		return (size() + 1 == name.size()) && isPrefixOf(name);
	}
	
	std::string Name::toString(bool addPrefix) const{
		std::string str;
		if (addPrefix && isAbsolute_) str += "::";
		
		for (CItType it = list_.begin(); it != list_.end(); ++it) {
			if (it != list_.begin()) str += "::";
			str += *it;
		}
		return str;
	}
	
	std::string Name::genString() const{
		std::string str;
		for(CItType it = list_.begin(); it != list_.end(); ++it){
			if(it != list_.begin()) str += "__";
			str += *it;
		}
		return str;
	}
	
	std::string Name::first() const{
		return list_.front();
	}
	
	std::string Name::last() const{
		assert(list_.back() == revAt(0)
		&& "The last element must be the first element when the list is in reverse order");
		return list_.back();
	}
	
	std::string Name::onlyElement() const{
		assert(list_.size() == 1
		&& "Getting the only element of a name means there must be exactly one element");
		return list_.front();
	}
	
	bool Name::empty() const{
		return list_.empty();
	}
	
	std::size_t Name::size() const{
		return list_.size();
	}
	
	std::string Name::at(std::size_t i) const{
		assert(i < list_.size());
		return list_.at(i);
	}
	
	std::string Name::revAt(std::size_t i) const{
		assert(i < list_.size());
		return list_.at(list_.size() - i - 1);
	}
	
	Name Name::substr(std::size_t substrSize) const{
		return Name(*this, substrSize);
	}
	
	Name Name::concat(const Name& suffix) const{
		return Name(*this, suffix);
	}
	
	Name Name::append(const std::string& suffix) const{
		return Name(*this, suffix);
	}
	
	Name Name::getPrefix() const {
		assert(!empty());
		return substr(size() - 1);
	}
	
	Name::ListType::iterator Name::begin() {
		return list_.begin();
	}
	
	Name::ListType::iterator Name::end() {
		return list_.end();
	}
	
	Name::ListType::const_iterator Name::begin() const {
		return list_.begin();
	}
	
	Name::ListType::const_iterator Name::end() const {
		return list_.end();
	}
	
	std::string CanonicalizeMethodName(const std::string& name) {
		std::string canonicalName;
		bool preserveUnderscores = true;
		
		for (size_t i = 0; i < name.size(); i++) {
			const auto c = name.at(i);
			
			// Preserve underscores at the beginning of the name.
			if (preserveUnderscores && c == '_') {
				canonicalName += "_";
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
	
}
	
