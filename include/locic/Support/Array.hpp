#ifndef LOCIC_SUPPORT_ARRAY_HPP
#define LOCIC_SUPPORT_ARRAY_HPP

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <locic/Support/Copy.hpp>

namespace locic{
	
	/**
	 * \brief Array Type
	 * 
	 * This array type contains a statically allocated portion and
	 * a dynamically allocated portion. The idea is that the statically
	 * allocated portion is used in the vast majority of cases, avoiding
	 * potentially a large number expensive heap allocations, while still
	 * allowing abnormally large arrays to grow using the heap.
	 * 
	 * Given that std::vector<T> has only a dynamically allocated
	 * it can't be used (although is used by this array internally),
	 * hence a custom array type was needed.
	 * 
	 * 'BaseSize' gives the size of the statically allocated portion in
	 * terms of sizeof(T).
	 */
	template <typename T, size_t BaseSize>
	class Array {
		public:
			typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type StaticArrayElementType;
			typedef StaticArrayElementType StaticArrayType[BaseSize];
			typedef T value_type;
			typedef T* iterator;
			typedef const T* const_iterator;
			typedef size_t size_type;
			typedef T& reference;
			typedef const T& const_reference;
			
			inline Array() : size_(0) { }
			
			Array(std::initializer_list<T> list)
			: size_(0) {
				reserve(list.size());
				for (auto& element: list) {
					push_back(std::move(element));
				}
			}
			
			~Array() {
				clear();
			}
			
			Array(Array<T, BaseSize>&& other)
			: size_(0) {
				if (other.using_static_space()) {
					for (size_t i = 0; i < other.size(); i++) {
						push_back(std::move(other[i]));
					}
				} else {
					vector_ = std::move(other.vector_);
				}
				size_ = other.size();
				other.clear();
			}
			
			Array<T, BaseSize>& operator=(Array<T, BaseSize>&& other) {
				clear();
				if (other.using_static_space()) {
					for (size_t i = 0; i < other.size(); i++) {
						push_back(std::move(other[i]));
					}
				} else {
					vector_ = std::move(other.vector_);
				}
				size_ = other.size();
				other.clear();
				return *this;
			}
			
			Array<T, BaseSize> copy() const {
				return Array<T, BaseSize>(*this);
			}
			
			inline bool empty() const {
				return size() == 0;
			}
			
			inline bool using_static_space() const {
				return size() <= BaseSize;
			}
			
			inline bool using_dynamic_space() const {
				return size() > BaseSize;
			}
			
			inline T* data() {
				if (using_static_space()) {
					return &(static_index(0));
				} else {
					return vector_.data();
				}
			}
			
			inline const T* data() const {
				if (using_static_space()) {
					return &(static_index(0));
				} else {
					return vector_.data();
				}
			}
			
			inline size_t size() const {
				return size_;
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
			
			inline T& operator[](const size_t index) {
				assert(index < size());
				
				if (using_static_space()) {
					return static_index(index);
				} else {
					return vector_[index];
				}
			}
			
			inline const T& operator[](const size_t index) const {
				assert(index < size());
				
				if (using_static_space()) {
					return static_index(index);
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
				if (size() == BaseSize) {
					vector_.reserve(size() + 1);
					
					// Move all objects into vector.
					for (size_t i = 0; i < size(); i++) {
						vector_.push_back(std::move(static_index(i)));
					}
					
					// Destroy moved-from objects.
					for (size_t i = 0; i < size(); i++) {
						const size_t j = size() - i - 1;
						static_index(j).~T();
					}
				}
				
				if (size() < BaseSize) {
					new (&(static_index(size()))) T(std::move(value));
				} else {
					vector_.push_back(std::move(value));
				}
				
				size_++;
			}
			
			void pop_back() {
				assert(!empty());
				if (size() <= BaseSize) {
					static_index(size() - 1).~T();
				} else {
					vector_.pop_back();
				}
				
				size_--;
				
				if (size() == BaseSize) {
					// Move all objects into static space.
					for (size_t i = 0; i < size(); i++) {
						// TODO: handle this throwing!
						new(&(static_index(i))) T(std::move(vector_[i]));
					}
					
					// Destroy moved-from objects.
					vector_.clear();
				}
			}
			
			void clear() {
				if (using_static_space()) {
					for (size_t i = 0; i < size(); i++) {
						const size_t j = size() - i - 1;
						static_index(j).~T();
					}
				} else {
					vector_.clear();
				}
				size_ = 0;
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
			
		private:
			Array(const Array<T, BaseSize>& other)
			: size_(0) {
				reserve(other.size());
				for (size_t i = 0; i < other.size(); i++) {
					push_back(copyObject(other[i]));
				}
			}
			
			Array<T, BaseSize>& operator=(const Array<T, BaseSize>&) = delete;
			
			inline T& static_index(const size_t index) {
				assert(index < BaseSize);
				return castRef(array_[index]);
			}
			
			inline const T& static_index(const size_t index) const {
				assert(index < BaseSize);
				return castRef(array_[index]);
			}
			
			static inline T& castRef(StaticArrayElementType& element) {
				return *(reinterpret_cast<T*>(&element));
			}
			
			static inline const T& castRef(const StaticArrayElementType& element) {
				return *(reinterpret_cast<const T*>(&element));
			}
			
			size_t size_;
			StaticArrayType array_;
			std::vector<T> vector_;
			
	};

}

#endif
