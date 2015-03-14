#ifndef LOCIC_SUPPORT_HEAPARRAY_HPP
#define LOCIC_SUPPORT_HEAPARRAY_HPP

#include <initializer_list>
#include <vector>

#include <locic/Support/Array.hpp>
#include <locic/Support/Copy.hpp>

namespace locic{
	
	template <typename T>
	class HeapArray {
	public:
		using ArrayType = std::vector<T>;
		using const_iterator = typename ArrayType::const_iterator;
		using const_reference = const T&;
		using iterator = typename ArrayType::iterator;
		using reference = T&;
		using size_type = size_t;
		using value_type = T;
		
		inline HeapArray() : array_() { }
		
		HeapArray(std::initializer_list<T> list) {
			reserve(list.size());
			for (auto& element: list) {
				push_back(std::move(element));
			}
		}
		
		template <size_t N>
		HeapArray(Array<T, N> array) {
			reserve(array.size());
			for (auto& element: array) {
				push_back(std::move(element));
			}
		}
		
		HeapArray(HeapArray<T>&&) = default;
		
		HeapArray& operator=(HeapArray<T>&&) = default;
		
		HeapArray<T> copy() const {
			HeapArray<T> result;
			result.reserve(size());
			for (const auto& value: *this) {
				result.push_back(copyObject(value));
			}
			return result;
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
		
		T& front() {
			return array_.front();
		}
		
		const T& front() const {
			return array_.front();
		}
		
		T& back() {
			return array_.back();
		}
		
		const T& back() const {
			return array_.back();
		}
		
		size_t size() const {
			return array_.size();
		}
		
		bool empty() const {
			return array_.empty();
		}
		
		T* data() {
			return array_.data();
		}
		
		const T* data() const {
			return array_.data();
		}
		
		T& operator[](const size_t index) {
			assert(index < size());
			return array_[index];
		}
		
		const T& operator[](const size_t index) const {
			assert(index < size());
			return array_[index];
		}
		
		T& at(const size_t index) {
			assert(index < size());
			return array_.at(index);
		}
		
		const T& at(const size_t index) const {
			assert(index < size());
			return array_.at(index);
		}
		
		void reserve(const size_t newCapacity) {
			array_.reserve(newCapacity);
		}
		
		void push_back(T value) {
			array_.push_back(std::move(value));
		}
		
		void pop_back() {
			array_.pop_back();
		}
		
		bool operator==(const HeapArray<T>& other) const {
			return array_ == other.array_;
		}
		
		bool operator!=(const HeapArray<T>& other) const {
			return array_ != other.array_;
		}
		
	private:
		ArrayType array_;
		
	};
	
}

#endif
