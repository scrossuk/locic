#ifndef LOCIC_NAME_HPP
#define LOCIC_NAME_HPP

#include <cassert>
#include <list>
#include <string>

namespace Locic{

	class Name{
		typedef std::vector<std::string> ListType;
		typedef ListType::const_iterator CItType;
	
		public:
			inline Name(){ }
			
			inline explicit Name(const std::string& str){
				list_.push_back(str);
			}
			
			inline Name(const Name& prefix, const std::string& suffix){
				for(std::size_t i = 0; i < prefix.size(); i++){
					list_.push_back(prefix.at(i));
				}
				list_.push_back(suffix);
			}
			
			inline static Name Relative(const std::string& name){
				// TODO!
				return Name(name);
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
			
			inline bool isPrefixOf(const Name& name) const{
				if(size() >= name.size()) return false;
				for(std::size_t i = 0; i < size(); i++){
					if(at(i) != name.at(i)){
						return false;
					}
				}
				return true;
			}
			
			inline std::string toString() const{
				std::string str;
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
				return list_.back();
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
			
		private:
			ListType list_;
			
	};

}

#endif
