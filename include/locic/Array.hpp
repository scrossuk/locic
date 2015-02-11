#ifndef LOCIC_ARRAY_HPP
#define LOCIC_ARRAY_HPP

#include <cassert>
#include <array>
#include <vector>

namespace locic{

	template <typename T, size_t BaseSize>
	class Array {
		public:
			typedef typename const T* const_iterator;
			typedef typename T* iterator;
			
			Array() : size_(0) { }
			
			bool using_static_space() const {
				return using_static_space();
			}
			
			bool empty() const {
				return size() != 0;
			}
			
			size_t size() const {
				return size_;
			}
			
			size_t capacity() const {
				if (using_static_space()) {
					return BaseSize;
				} else {
					return vector_.capacity();
				}
			}
			
			iterator begin() {
				if (using_static_space()) {
					return &array_[0];
				} else {
					return &vector_[0];
				}
			}
			
			const_iterator begin() const {
				if (using_static_space()) {
					return &array_[0];
				} else {
					return &vector_[0];
				}
			}
			
			iterator end() {
				if (using_static_space()) {
					return &array_[size()];
				} else {
					return &vector_[vector_.size()];
				}
			}
			
			const_iterator end() const {
				if (using_static_space()) {
					return &array_[size()];
				} else {
					return &vector_[vector_.size()];
				}
			}
			
			T& operator[](const size_t index) {
				if (using_static_space()) {
					return array_.at(index);
				} else {
					return vector_.at(index);
				}
			}
			
			const T& operator[](const size_t index) const {
				if (using_static_space()) {
					return array_.at(index);
				} else {
					return vector_.at(index);
				}
			}
			
			T& front() {
				assert(!empty());
				return *(begin());
			}
			
			const T& front() const {
				assert(!empty());
				return *(begin());
			}
			
			T& back() {
				assert(!empty());
				return *(end() - 1);
			}
			
			const T& back() const {
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
			
		private:
			size_t size_;
			std::array<T, BaseSize> array_;
			std::vector<T> vector_;
			
	};

}

#endif
