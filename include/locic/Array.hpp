#ifndef LOCIC_ARRAY_HPP
#define LOCIC_ARRAY_HPP

#include <array>
#include <cassert>
#include <initializer_list>
#include <vector>

namespace locic{

	template <typename T, size_t BaseSize>
	class Array {
		public:
			typedef const T* const_iterator;
			typedef T* iterator;
			typedef T value_type;
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
			
			Array(Array<T, BaseSize>&&) = default;
			Array<T, BaseSize>& operator=(Array<T, BaseSize>&&) = default;
			
			Array<T, BaseSize> copy() const {
				return Array<T, BaseSize>(*this);
			}
			
			inline bool using_static_space() const {
				return size_ <= BaseSize;
			}
			
			inline bool empty() const {
				return size() == 0;
			}
			
			inline T* data() {
				return begin();
			}
			
			inline const T* data() const {
				return begin();
			}
			
			inline size_t size() const {
				return size_;
			}
			
			inline size_t capacity() const {
				if (using_static_space()) {
					return BaseSize;
				} else {
					return vector_.capacity();
				}
			}
			
			inline iterator begin() {
				if (using_static_space()) {
					return &array_[0];
				} else {
					return &vector_[0];
				}
			}
			
			inline const_iterator begin() const {
				if (using_static_space()) {
					return &array_[0];
				} else {
					return &vector_[0];
				}
			}
			
			inline iterator end() {
				if (using_static_space()) {
					return &array_[size()];
				} else {
					return &vector_[vector_.size()];
				}
			}
			
			inline const_iterator end() const {
				if (using_static_space()) {
					return &array_[size()];
				} else {
					return &vector_[vector_.size()];
				}
			}
			
			inline T& operator[](const size_t index) {
				if (using_static_space()) {
					return array_[index];
				} else {
					return vector_[index];
				}
			}
			
			inline const T& operator[](const size_t index) const {
				if (using_static_space()) {
					return array_[index];
				} else {
					return vector_[index];
				}
			}
			
			inline T& at(const size_t index) {
				if (using_static_space()) {
					return array_.at(index);
				} else {
					return vector_.at(index);
				}
			}
			
			inline const T& at(const size_t index) const {
				if (using_static_space()) {
					return array_.at(index);
				} else {
					return vector_.at(index);
				}
			}
			
			inline T& front() {
				assert(!empty());
				return *(begin());
			}
			
			inline const T& front() const {
				assert(!empty());
				return *(begin());
			}
			
			inline T& back() {
				assert(!empty());
				return *(end() - 1);
			}
			
			inline const T& back() const {
				assert(!empty());
				return *(end() - 1);
			}
			
			void reserve(const size_t space) {
				if (space <= BaseSize) {
					return;
				}
				
				vector_.reserve(space);
			}
			
			void push_back(T value) {
				if ((size() + 1) <= BaseSize) {
					array_[size()] = std::move(value);
				} else {
					if (size() == BaseSize) {
						vector_.reserve(BaseSize + 1);
						for (auto& element: array_) {
							vector_.push_back(std::move(element));
						}
					}
					vector_.push_back(std::move(value));
				}
				size_++;
			}
			
			void pop_back() {
				assert(!empty());
				if ((size() - 1) <= BaseSize) {
					if (using_static_space()) {
						array_[size() - 1] = T();
					} else {
						for (size_t i = 0; i < size() - 1; i++) {
							array_[i] = std::move(vector_[i]);
						}
						vector_.clear();
					}
				} else {
					vector_.pop_back();
				}
				size_--;
			}
			
			void clear() {
				for (size_t i = 0; i < BaseSize; i++) {
					array_[i] = T();
				}
				vector_.clear();
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
			Array(const Array<T, BaseSize>&) = default;
			Array<T, BaseSize>& operator=(const Array<T, BaseSize>&) = delete;
			
			size_t size_;
			std::array<T, BaseSize> array_;
			std::vector<T> vector_;
			
	};

}

#endif
