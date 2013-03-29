#ifndef LOCIC_NAME_HPP
#define LOCIC_NAME_HPP

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

namespace Locic{

	class Name{
		typedef std::vector<std::string> ListType;
		typedef ListType::const_iterator CItType;
	
		public:
			inline Name()
				: isAbsolute_(true){ }
			
			inline explicit Name(bool isNameAbsolute)
				: isAbsolute_(isNameAbsolute){ }
			
			inline Name(const Name& name, size_t substrSize)
				: isAbsolute_(name.isAbsolute()){
				
				assert(substrSize <= name.size());
				
				for(std::size_t i = 0; i < substrSize; i++){
					list_.push_back(name.at(i));
				}	
			}
			
			inline Name(const Name& prefix, const std::string& suffix)
				: isAbsolute_(prefix.isAbsolute()){
				for(std::size_t i = 0; i < prefix.size(); i++){
					list_.push_back(prefix.at(i));
				}
				
				// No member of a name can be an empty string.
				if(suffix.size() > 0){
					list_.push_back(suffix);
				}
			}
			
			inline Name(const Name& prefix, const Name& suffix)
				: isAbsolute_(prefix.isAbsolute()){
				for(std::size_t i = 0; i < prefix.size(); i++){
					list_.push_back(prefix.at(i));
				}
				for(std::size_t i = 0; i < suffix.size(); i++){
					list_.push_back(suffix.at(i));
				}
			}
			
			inline static Name Absolute(){
				return Name(true);
			}
			
			inline static Name Relative(){
				return Name(false);
			}
			
			inline bool operator==(const Name& name) const{
				if(size() != name.size()) return false;
				for(std::size_t i = 0; i < size(); i++){
					if(at(i) != name.at(i)){
						return false;
					}
				}
				return true;
			}
			
			inline Name operator+(const std::string& name) const{
				return Name(*this, name);
			}
			
			inline Name makeAbsolute(const Name& name) const{
				assert(isAbsolute());
				assert(!name.empty());
				if(name.isAbsolute()){
					return name;
				}else{
					return concat(name);
				}
			}
			
			inline bool isRelative() const{
				return !isAbsolute_;
			}
			
			inline bool isAbsolute() const{
				return isAbsolute_;
			}
			
			inline bool isPrefixOf(const Name& name) const{
				if(size() >= name.size()) return false;
				for(std::size_t i = 0; i < size(); i++){
					if(at(i) != name.at(i)){
						return false;
					}
				}
				return true;
			}
			
			inline bool isExactPrefixOf(const Name& name) const{
				return (size() + 1 == name.size()) && isPrefixOf(name);
			}
			
			inline std::string toString() const{
				std::string str;
				if(isAbsolute_) str += "::";
				
				for(CItType it = list_.begin(); it != list_.end(); ++it){
					if(it != list_.begin()) str += "::";
					str += *it;
				}
				return str;
			}
			
			inline std::string genString() const{
				std::string str;
				for(CItType it = list_.begin(); it != list_.end(); ++it){
					if(it != list_.begin()) str += "__";
					str += *it;
				}
				return str;
			}
			
			inline std::string first() const{
				return list_.front();
			}
			
			inline std::string last() const{
				assert(list_.back() == revAt(0)
					&& "The last element must be the first element when the list is in reverse order");
				return list_.back();
			}
			
			inline std::string onlyElement() const{
				assert(list_.size() == 1
					&& "Getting the only element of a name means there must be exactly one element");
				return list_.front();
			}
			
			inline bool empty() const{
				return list_.empty();
			}
			
			inline std::size_t size() const{
				return list_.size();
			}
			
			inline std::string at(std::size_t i) const{
				assert(i < list_.size());
				return list_.at(i);
			}
			
			inline std::string revAt(std::size_t i) const{
				assert(i < list_.size());
				return list_.at(list_.size() - i - 1);
			}
			
			inline Name substr(std::size_t substrSize) const{
				return Name(*this, substrSize);
			}
			
			inline Name concat(const Name& suffix) const{
				return Name(*this, suffix);
			}
			
			inline Name append(const std::string& suffix) const{
				return Name(*this, suffix);
			}
			
		private:
			bool isAbsolute_;
			ListType list_;
			
	};

}

#endif
