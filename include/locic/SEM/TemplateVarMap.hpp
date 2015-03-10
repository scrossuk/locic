#ifndef LOCIC_SEM_TEMPLATEVARMAP_HPP
#define LOCIC_SEM_TEMPLATEVARMAP_HPP

#include <cassert>
#include <cstddef>

#include <locic/Support/Array.hpp>

namespace locic {

	namespace SEM {
	
		class TemplateVar;
		class Type;
		
		constexpr size_t TemplateVarMapBaseSize = 8;
		
		/**
		 * \brief Template Var Map
		 * 
		 * An assignment of type values for template variables.
		 * 
		 * This type uses an underlying array and performs linear
		 * searches; while this is technically an O(n) cost
		 * the number of map keys are almost always very small
		 * (i.e. < 10) and hence this avoids unnecessary heap
		 * allocations and has better cache behaviour.
		 */
		class TemplateVarMap {
			public:
				using key_type = const TemplateVar*;
				using mapped_type = const Type*;
				using value_type = std::pair<key_type, mapped_type>;
				using ArrayType = Array<value_type, TemplateVarMapBaseSize>;
				using iterator = ArrayType::iterator;
				using const_iterator = ArrayType::const_iterator;
				
				TemplateVarMap() { }
				
				TemplateVarMap copy() const {
					TemplateVarMap newMap;
					newMap.array_ = array_.copy();
					return newMap;
				}
				
				bool empty() const {
					return array_.empty();
				}
				
				size_t size() const{
					return array_.size();
				}
				
				iterator begin() {
					return array_.begin();
				}
				
				const_iterator begin() const {
					return array_.begin();
				}
				
				iterator end() {
					return array_.end();
				}
				
				const_iterator end() const {
					return array_.end();
				}
				
				mapped_type& at(const key_type& key) {
					auto it = find(key);
					assert(it != end());
					return it->second;
				}
				
				const mapped_type& at(const key_type& key) const {
					const auto it = find(key);
					assert(it != end());
					return it->second;
				}
				
				void insert(value_type value) {
					assert(find(value.first) == end());
					array_.push_back(std::move(value));
				}
				
				iterator find(const key_type& key) {
					for (ArrayType::iterator it = array_.begin(); it != array_.end(); ++it) {
						if (it->first == key) {
							return it;
						}
					}
					return array_.end();
				}
				
				const_iterator find(const key_type& key) const {
					for (ArrayType::const_iterator it = array_.begin(); it != array_.end(); ++it) {
						if (it->first == key) {
							return it;
						}
					}
					return array_.end();
				}
				
			private:
				ArrayType array_;
				
		};
		
	}
	
}

#endif
