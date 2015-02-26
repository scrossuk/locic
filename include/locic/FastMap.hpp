#ifndef LOCIC_FASTMAP_HPP
#define LOCIC_FASTMAP_HPP

#include <cassert>
#include <map>

#include <locic/Copy.hpp>

namespace locic {
	
	template <typename Key, typename Value>
	class FastMap {
	public:
		using MapType = std::map<Key, Value>;
		using iterator = typename MapType::iterator;
		using const_iterator = typename MapType::const_iterator;
		
		FastMap() { }
		
		FastMap(FastMap<Key, Value>&&) = default;
		FastMap<Key, Value>& operator=(FastMap<Key, Value>&&) = default;
		
		FastMap<Key, Value> copy() const {
			FastMap<Key, Value> newMap;
			for (const auto& itemPair: *this) {
				newMap.insert(std::make_pair(copyObject(itemPair.first), copyObject(itemPair.second)));
			}
			return newMap;
		}
		
		iterator begin() {
			return map_.begin();
		}
		
		const_iterator begin() const {
			return map_.begin();
		}
		
		iterator end() {
			return map_.end();
		}
		
		const_iterator end() const {
			return map_.end();
		}
		
		Value& operator[](const Key& key) {
			return map_[key];
		}
		
		const Value& operator[](const Key& key) const {
			return map_[key];
		}
		
		Value& at(const Key& key) {
			return map_.at(key);
		}
		
		const Value& at(const Key& key) const {
			return map_.at(key);
		}
		
		std::pair<iterator, bool> insert(std::pair<Key, Value> pair) {
			return map_.insert(std::move(pair));
		}
		
		iterator find(const Key& key) {
			return map_.find(key);
		}
		
		const_iterator find(const Key& key) const {
			return map_.find(key);
		}
		
		std::size_t size() const{
			return map_.size();
		}
		
		bool empty() const{
			return map_.empty();
		}
		
		void clear() {
			map_.clear();
		}
		
	private:
		FastMap(const FastMap<Key, Value>&) = delete;
		FastMap<Key, Value>& operator=(const FastMap<Key, Value>&) = delete;
		
		MapType map_;
		
	};
	
}

#endif
