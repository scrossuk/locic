#ifndef LOCIC_SUPPORT_ARRAY_HPP
#define LOCIC_SUPPORT_ARRAY_HPP

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <locic/Support/Copy.hpp>
#include <locic/Support/Hasher.hpp>

namespace locic{
	
	template <typename T, size_t BaseSize>
	class ArrayStaticData {
	public:
		typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type element_type;
		typedef element_type array_type[BaseSize];
		
		inline ArrayStaticData()
		: size_(0) { }
		
		void set_static_size(const size_t new_size) {
			size_ = new_size;
		}
		
		size_t get_static_size() const {
			return size_;
		}
		
		inline T& static_index(const size_t index) {
			assert(index < BaseSize);
			return castRef(array_[index]);
		}
		
		inline const T& static_index(const size_t index) const {
			assert(index < BaseSize);
			return castRef(array_[index]);
		}
		
	private:
		static inline T& castRef(element_type& element) {
			return *(reinterpret_cast<T*>(&element));
		}
		
		static inline const T& castRef(const element_type& element) {
			return *(reinterpret_cast<const T*>(&element));
		}
		
		size_t size_;
		array_type array_;
		
	};
	
	template <typename T>
	class ArrayStaticData<T, 0> {
	public:
		void set_static_size(const size_t /*new_size*/) { }
		
		size_t get_static_size() const {
			return 0;
		}
		
		inline T& static_index(const size_t /*index*/) {
			assert(false && "Invalid index for zero-sized static array.");
			return *(static_cast<T*>(nullptr));
		}
		
		inline const T& static_index(const size_t /*index*/) const {
			assert(false && "Invalid index for zero-sized static array.");
			return *(static_cast<T*>(nullptr));
		}
		
	};
	
	/**
	 * \brief Array Type
	 * 
	 * This array type contains a statically allocated portion and
	 * a dynamically allocated portion. The idea is that the statically
	 * allocated portion is used in the vast majority of cases, avoiding
	 * potentially a large number expensive heap allocations, while still
	 * allowing abnormally large arrays to grow using the heap.
	 * 
	 * Given that std::vector<T> has only dynamically allocated storage
	 * it can't be used (although is used by this array internally),
	 * hence a custom array type was needed.
	 * 
	 * 'BaseSize' gives the size of the statically allocated portion in
	 * terms of sizeof(T).
	 */
	template <typename T, size_t BaseSize>
	class Array: private ArrayStaticData<T, BaseSize> {
		public:
			typedef T value_type;
			typedef T* iterator;
			typedef const T* const_iterator;
			typedef size_t size_type;
			typedef T& reference;
			typedef const T& const_reference;
			
			inline Array() { }
			
			Array(std::initializer_list<T> list) {
				reserve(list.size());
				for (auto& element: list) {
					push_back(std::move(element));
				}
			}
			
			~Array() {
				clear();
			}
			
			template <size_t OtherSize>
			Array(Array<T, OtherSize>&& other) {
				set_internal_data(std::move(other));
			}
			
			template <size_t OtherSize>
			Array<T, BaseSize>& operator=(Array<T, OtherSize>&& other) {
				assert(static_cast<void*>(this) != static_cast<void*>(&other));
				set_internal_data(std::move(other));
				return *this;
			}
			
			Array<T, BaseSize> copy() const {
				return Array<T, BaseSize>(*this);
			}
			
			inline bool empty() const {
				return size() == 0;
			}
			
			inline bool using_static_space() const {
				return this->get_static_size() > 0 ||
					(BaseSize > 0 && vector_.empty());
			}
			
			inline bool using_dynamic_space() const {
				return !using_static_space();
			}
			
			std::vector<T>& internal_dynamic_array() {
				return vector_;
			}
			
			inline T* data() {
				if (using_static_space()) {
					return &(this->static_index(0));
				} else {
					return vector_.data();
				}
			}
			
			inline const T* data() const {
				if (using_static_space()) {
					return &(this->static_index(0));
				} else {
					return vector_.data();
				}
			}
			
			inline size_t size() const {
				if (this->get_static_size() > 0) {
					assert(this->get_static_size() <= BaseSize);
					return this->get_static_size();
				} else {
					return vector_.size();
				}
			}
			
			inline size_t capacity() const {
				return vector_.capacity() > BaseSize ? vector_.capacity() : BaseSize;
			}
			
			inline iterator begin() {
				return data();
			}
			
			inline const_iterator begin() const {
				return data();
			}
			
			inline iterator end() {
				return data() + size();
			}
			
			inline const_iterator end() const {
				return data() + size();
			}
			
			bool contains(const T& value) const {
				for (const auto& element: *this) {
					if (value == element) {
						return true;
					}
				}
				
				return false;
			}
			
			inline T& operator[](const size_t index) {
				assert(index < size());
				
				if (using_static_space()) {
					return this->static_index(index);
				} else {
					return vector_[index];
				}
			}
			
			inline const T& operator[](const size_t index) const {
				assert(index < size());
				
				if (using_static_space()) {
					return this->static_index(index);
				} else {
					return vector_[index];
				}
			}
			
			inline T& at(const size_t index) {
				if (index >= size()) {
					throw std::out_of_range("Array::at()");
				}
				
				return (*this)[index];
			}
			
			inline const T& at(const size_t index) const {
				if (index >= size()) {
					throw std::out_of_range("Array::at()");
				}
				
				return (*this)[index];
			}
			
			inline T& front() {
				assert(!empty());
				return (*this)[0];
			}
			
			inline const T& front() const {
				assert(!empty());
				return (*this)[0];
			}
			
			inline T& back() {
				assert(!empty());
				return (*this)[size() - 1];
			}
			
			inline const T& back() const {
				assert(!empty());
				return (*this)[size() - 1];
			}
			
			void reserve(const size_t space) {
				if (space > BaseSize) {
					vector_.reserve(space);
				}
			}
			
			void push_back(T value) {
				const size_t old_size = size();
				
				if (old_size == BaseSize) {
					vector_.reserve(old_size + 1);
					
					// Move all objects into vector.
					for (size_t i = 0; i < old_size; i++) {
						vector_.push_back(std::move(this->static_index(i)));
					}
					
					// Destroy moved-from objects.
					for (size_t i = 0; i < old_size; i++) {
						const size_t j = old_size - i - 1;
						this->static_index(j).~T();
					}
					
					this->set_static_size(0);
				}
				
				if (old_size < BaseSize) {
					new (&(this->static_index(old_size))) T(std::move(value));
					this->set_static_size(old_size + 1);
				} else {
					assert(this->get_static_size() == 0);
					vector_.push_back(std::move(value));
				}
			}
			
			void pop_back() {
				assert(!empty());
				const size_t old_size = size();
				
				if (old_size <= BaseSize) {
					this->static_index(old_size - 1).~T();
					this->set_static_size(old_size - 1);
				} else {
					assert(this->get_static_size() == 0);
					vector_.pop_back();
				}
				
				const size_t new_size = old_size - 1;
				
				if (new_size == BaseSize) {
					// Move all objects into static space.
					for (size_t i = 0; i < new_size; i++) {
						// TODO: handle this throwing!
						new(&(this->static_index(i))) T(std::move(vector_[i]));
					}
					
					// Destroy moved-from objects.
					vector_.clear();
					
					this->set_static_size(new_size);
				}
			}
			
			void clear() {
				for (size_t i = 0; i < this->get_static_size(); i++) {
					const size_t j = this->get_static_size() - i - 1;
					this->static_index(j).~T();
				}
				this->set_static_size(0);
				vector_.clear();
			}
			
			inline bool operator==(const Array<T, BaseSize>& other) const {
				if (size() != other.size()) {
					return false;
				}
				
				for (size_t i = 0; i < size(); i++) {
					if (!((*this)[i] == other[i])) {
						return false;
					}
				}
				
				return true;
			}
			
			inline bool operator!=(const Array<T, BaseSize>& other) const {
				if (size() != other.size()) {
					return true;
				}
				
				for (size_t i = 0; i < size(); i++) {
					if ((*this)[i] != other[i]) {
						return true;
					}
				}
				
				return false;
			}
			
			inline bool operator<(const Array<T, BaseSize>& other) const {
				if (size() != other.size()) {
					return size() < other.size();
				}
				
				for (size_t i = 0; i < size(); i++) {
					if ((*this)[i] < other[i]) {
						return true;
					}
					if (other[i] < (*this)[i]) {
						return false;
					}
				}
				
				return false;
			}
			
			inline bool operator>(const Array<T, BaseSize>& other) const {
				if (size() != other.size()) {
					return size() > other.size();
				}
				
				for (size_t i = 0; i < size(); i++) {
					if ((*this)[i] > other[i]) {
						return true;
					}
					if (other[i] > (*this)[i]) {
						return false;
					}
				}
				
				return false;
			}
			
			size_t hash() const {
				Hasher hasher;
				for (const auto& element: *this) {
					hasher.add(element);
				}
				return hasher.get();
			}
			
		private:
			Array(const Array<T, BaseSize>& other) {
				reserve(other.size());
				for (size_t i = 0; i < other.size(); i++) {
					push_back(copyObject(other[i]));
				}
			}
			
			template <size_t OtherSize>
			void set_internal_data(Array<T, OtherSize>&& other) {
				clear();
				
				assert(size() == 0);
				assert(vector_.empty());
				assert(this->get_static_size() == 0);
				
				const size_t other_size = other.size();
				
				const bool can_allocate_statically = other_size <= BaseSize;
				if (other.using_static_space() || can_allocate_statically) {
					if (can_allocate_statically) {
						for (size_t i = 0; i < other_size; i++) {
							push_back(std::move(other[i]));
						}
						this->set_static_size(other_size);
						assert(vector_.empty());
					} else {
						vector_.reserve(other_size);
						for (size_t i = 0; i < other_size; i++) {
							vector_.push_back(std::move(other[i]));
						}
					}
				} else {
					vector_ = std::move(other.internal_dynamic_array());
				}
				assert(size() == other_size);
				other.clear();
			}
			
			Array<T, BaseSize>& operator=(const Array<T, BaseSize>&) = delete;
			
			std::vector<T> vector_;
			
	};

}

#endif
