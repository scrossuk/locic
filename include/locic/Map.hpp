#ifndef LOCIC_MAP_HPP
#define LOCIC_MAP_HPP

#include <cassert>
#include <map>
#include <string>
#include <boost/utility.hpp>
#include <locic/Optional.hpp>

namespace locic{

	template <typename Key, typename Value>
	class Map{
		private:
			typedef std::map<Key, Value> MapType;
			typedef typename MapType::const_iterator CItType;
			typedef typename MapType::iterator ItType;
	
		public:
			class Pair{
				public:
					Pair(const Key& k, const Value& v)
						: key_(k), value_(v){ }
					
					Key key() const{
						return key_;
					}
					
					Value value() const{
						return value_;
					}
					
				private:
					Key key_;
					Value value_;
				
			};
			
			class Range{
				public:
					Range(const CItType& begin,
						const CItType& end)
						: begin_(begin), end_(end){ }
					
					bool empty() const{
						return begin_ == end_;
					}
					
					Pair front() const{
						assert(!empty());
						return Pair(begin_->first, begin_->second);
					}
					
					Pair back() const{
						assert(!empty());
						CItType end = end_;
						--end;
						return Pair(end->first, end->second);
					}
				
					void popFront(){
						assert(!empty());
						++begin_;
					}
					
					void popBack(){
						assert(!empty());
						--end_;
					}
			
				private:
					CItType begin_, end_;
			};
			
			Range range() const{
				return Range(map_.begin(), map_.end());
			}
		
			bool tryInsert(const Key& key, const Value& value){
				std::pair<ItType, bool> p = map_.insert(std::make_pair(key, value));
				return p.second;
			}
			
			void insert(const Key& key, const Value& value){
				const bool couldInsert = tryInsert(key, value);
				(void) couldInsert;
				assert(couldInsert);
			}
			
			void forceInsert(const Key& key, const Value& value){
				std::pair<ItType, bool> p = map_.insert(std::make_pair(key, value));
				if(!p.second){
					ItType it = p.first;
					it->second = value;
				}
			}
			
			Optional<Value> tryGet(const Key& key) const{
				CItType it = map_.find(key);
				if(it != map_.end()){
					return make_optional(it->second);
				}else{
					return Optional<Value>();
				}
			}
			
			Value get(const Key& key) const{
				Optional<Value> value = tryGet(key);
				assert(value);
				return *value;
			}
			
			bool has(const Key& key) const{
				return tryGet(key).hasValue();
			}
			
			std::size_t size() const{
				return map_.size();
			}
			
			bool empty() const{
				return map_.empty();
			}
			
			void insertRange(const Range& rangeToInsert){
				map_.insert(rangeToInsert.begin_, rangeToInsert.end_);
			}
			
			void clear() {
				map_.clear();
			}
			
		private:
			MapType map_;
		
	};
	
	template <typename Value>
	class StringMap: public Map<std::string, Value>{ };

}

#endif
