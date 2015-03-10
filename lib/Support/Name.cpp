#include <cassert>
#include <cstddef>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/Support/Array.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	Name::Name()
	: isAbsolute_(true){ }
	
	Name::Name(bool isNameAbsolute)
	: isAbsolute_(isNameAbsolute){ }
	
	Name::Name(const Name& name, size_t substrSize)
	: isAbsolute_(name.isAbsolute()) {
		assert(substrSize <= name.size());
		list_.reserve(substrSize);
		for(std::size_t i = 0; i < substrSize; i++){
			list_.push_back(name.at(i));
		}
	}
	
	Name::Name(const Name& prefix, const String& suffix)
	: isAbsolute_(prefix.isAbsolute()) {
		list_.reserve(prefix.size() + 1);
		for (std::size_t i = 0; i < prefix.size(); i++) {
			list_.push_back(prefix.at(i));
		}
		
		// No member of a name can be an empty string.
		if (suffix.size() > 0) {
			list_.push_back(suffix);
		}
	}
	
	Name::Name(const Name& prefix, const Name& suffix)
	: isAbsolute_(prefix.isAbsolute()) {
		list_.reserve(prefix.size() + suffix.size());
		for(std::size_t i = 0; i < prefix.size(); i++){
			list_.push_back(prefix.at(i));
		}
		for(std::size_t i = 0; i < suffix.size(); i++){
			list_.push_back(suffix.at(i));
		}
	}
	
	Name Name::Absolute() {
		return Name(true);
	}
	
	Name Name::Relative() {
		return Name(false);
	}
	
	Name Name::copy() const {
		Name newName;
		newName.list_ = list_.copy();
		return newName;
	}
	
	bool Name::operator==(const Name& name) const{
		if (size() != name.size()) return false;
		for (std::size_t i = 0; i < size(); i++) {
			if (at(i) != name.at(i)) {
				return false;
			}
		}
		return true;
	}
	
	bool Name::operator<(const Name& name) const{
		if (size() != name.size()) return size() < name.size();
		for (std::size_t i = 0; i < size(); i++) {
			if (at(i) != name.at(i)) {
				return at(i) < name.at(i);
			}
		}
		return false;
	}
	
	Name Name::operator+(const String& name) const{
		return Name(*this, name);
	}
	
	Name Name::makeAbsolute(const Name& name) const{
		assert(isAbsolute());
		assert(!name.empty());
		if(name.isAbsolute()) {
			return name.copy();
		} else {
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
			str += it->toString();
		}
		return str;
	}
	
	String Name::genString() const {
		assert(!empty());
		std::string str = list_.front().asStdString();
		for (CItType it = list_.begin() + 1; it != list_.end(); ++it) {
			str += "::";
			str += it->asStdString();
		}
		return String(list_.front().host(), std::move(str));
	}
	
	const String& Name::first() const{
		return list_.front();
	}
	
	const String& Name::last() const{
		assert(list_.back() == revAt(0)
		&& "The last element must be the first element when the list is in reverse order");
		return list_.back();
	}
	
	const String& Name::onlyElement() const{
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
	
	const String& Name::at(std::size_t i) const{
		assert(i < list_.size());
		return list_.at(i);
	}
	
	const String& Name::revAt(std::size_t i) const{
		assert(i < list_.size());
		return list_.at(list_.size() - i - 1);
	}
	
	Name Name::substr(std::size_t substrSize) const{
		return Name(*this, substrSize);
	}
	
	Name Name::concat(const Name& suffix) const{
		return Name(*this, suffix);
	}
	
	Name Name::append(const String& suffix) const{
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
	
	std::size_t Name::hash() const {
		std::size_t seed = 0;
		std::hash<String> stringHashFn;
		for (size_t i = 0; i < size(); i++) {
			boost::hash_combine(seed, stringHashFn(at(i)));
		}
		return seed;
	}
	
}
	
