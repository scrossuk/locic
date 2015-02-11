#ifndef LOCIC_STABLESET_HPP
#define LOCIC_STABLESET_HPP

#include <cassert>
#include <set>
#include <unordered_map>

namespace locic{

	template <typename Value>
	class StableSet {
		public:
			typedef std::set<Value> SetType;
			typedef typename SetType::const_iterator const_iterator;
			typedef typename SetType::iterator iterator;
			
			StableSet() { }
			
			bool empty() const {
				return valueSet_.empty();
			}
			
			size_t size() const {
				return valueSet_.size();
			}
			
			iterator begin() {
				return valueSet_.begin();
			}
			
			const_iterator begin() const {
				return valueSet_.begin();
			}
			
			iterator end() {
				return valueSet_.end();
			}
			
			const_iterator end() const {
				return valueSet_.end();
			}
			
			const_iterator find(const Value& key) const {
				const auto it = valuePointers_.find(&key);
				if (it != valuePointers_.end()) {
					return it->second;
				} else {
					return end();
				}
			}
			
			iterator find(const Value& key) {
				const auto it = valuePointers_.find(&key);
				if (it != valuePointers_.end()) {
					return it->second;
				} else {
					return end();
				}
			}
			
			std::pair<iterator, bool> insert(Value value) {
				const auto it = find(value);
				if (it != end()) {
					return std::make_pair(it, false);
				}
				
				const auto result = valueSet_.insert(std::move(value));
				assert(result.second);
				const auto valuePointer = &(*(result.first));
				valuePointers_.insert(std::make_pair(valuePointer, result.first));
				return result;
			}
			
		private:
			struct hashTypePtr {
				inline std::size_t operator()(const Value* const ptr) const {
					return ptr->hash();
				}
			};
			
			struct isEqualTypePtr {
				inline bool operator()(const Value* const a, const Value* const b) const {
					return *a == *b;
				}
			};
			
			std::unordered_map<const Value*, iterator, hashTypePtr, isEqualTypePtr> valuePointers_;
			SetType valueSet_;
		
	};

}

#endif
