#ifndef LOCIC_STABLESET_HPP
#define LOCIC_STABLESET_HPP

#include <cassert>
#include <unordered_map>

#include <locic/Support/Array.hpp>

namespace locic{
	
	template <typename Value>
	class StableSet {
	private:
		class SetArray;
	public:
		static constexpr size_t IndividualValueArraySize = 32;
		static constexpr size_t BaseNumberValueArrays = 16;
		typedef Array<SetArray, BaseNumberValueArrays> ValueArrays;
		
		StableSet() : nextValue_(0) { }
		
		~StableSet() {
			for (size_t i = 0; i < size(); i++) {
				const size_t j = size() - i - 1;
				iterator(arrays_, j).get()->~Value();
			}
		}
		
		class iterator {
		public:
			iterator(ValueArrays& argArrays, const size_t argId)
			: arrays_(argArrays), id_(argId) { }
			
			ValueArrays& arrays() {
				return arrays_;
			}
			
			const ValueArrays& arrays() const {
				return arrays_;
			}
			
			size_t id() const {
				return id_;
			}
			
			Value* get() {
				const size_t arrayId = (id() / IndividualValueArraySize);
				const size_t indexWithinArray = (id() % IndividualValueArraySize);
				return &(arrays_[arrayId][indexWithinArray]);
			}
			
			const Value* get() const {
				const size_t arrayId = (id() / IndividualValueArraySize);
				const size_t indexWithinArray = (id() % IndividualValueArraySize);
				return &(arrays_[arrayId][indexWithinArray]);
			}
			
			bool operator==(const iterator& other) const {
				return &(arrays()) == &(other.arrays()) && id() == other.id();
			}
			
			bool operator!=(const iterator& other) const {
				return !(*this == other);
			}
			
			Value& operator*() {
				return *(get());
			}
			
			const Value& operator*() const {
				return *(get());
			}
			
			iterator& operator++() {
				id_++;
				return *this;
			}
			
			iterator& operator--() {
				id_--;
				return *this;
			}
			
		private:
			ValueArrays& arrays_;
			size_t id_;
			
		};
		
		class const_iterator {
		public:
			const_iterator(const ValueArrays& argArrays, const size_t argId)
			: arrays_(argArrays), id_(argId) { }
			
			const_iterator(const iterator& it)
			: arrays_(it.arrays()), id_(it.id()) { }
			
			const ValueArrays& arrays() const {
				return arrays_;
			}
			
			size_t id() const {
				return id_;
			}
			
			const Value* get() const {
				const size_t arrayId = (id() / IndividualValueArraySize);
				const size_t indexWithinArray = (id() % IndividualValueArraySize);
				return &(arrays_[arrayId][indexWithinArray]);
			}
			
			bool operator==(const const_iterator& other) const {
				return &(arrays()) == &(other.arrays()) && id() == other.id();
			}
			
			bool operator!=(const const_iterator& other) const {
				return !(*this == other);
			}
			
			const Value& operator*() const {
				return *(get());
			}
			
			const_iterator& operator++() {
				id_++;
				return *this;
			}
			
			const_iterator& operator--() {
				id_--;
				return *this;
			}
			
		private:
			ValueArrays& arrays_;
			size_t id_;
			
		};
		
		bool empty() const {
			return nextValue_ == 0;
		}
		
		size_t size() const {
			return nextValue_;
		}
		
		iterator begin() {
			return iterator(arrays_, 0);
		}
		
		const_iterator begin() const {
			return const_iterator(arrays_, 0);
		}
		
		iterator end() {
			return iterator(arrays_, nextValue_);
		}
		
		const_iterator end() const {
			return const_iterator(arrays_, nextValue_);
		}
		
		iterator find(const Value& key) {
			const auto it = iteratorMap_.find(&key);
			if (it != iteratorMap_.end()) {
				return it->second;
			} else {
				return end();
			}
		}
		
		const_iterator find(const Value& key) const {
			const auto it = iteratorMap_.find(&key);
			if (it != iteratorMap_.end()) {
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
			
			const auto newIt = allocateValue(std::move(value));
			iteratorMap_.insert(std::make_pair(newIt.get(), newIt));
			return std::make_pair(newIt, true);
		}
		
		size_t unique_count() const {
			return iteratorMap_.size();
		}
		
		float load_factor() const {
			return iteratorMap_.load_factor();
		}
		
		float max_load_factor() const {
			return iteratorMap_.max_load_factor();
		}
		
		size_t bucket_count() const {
			return iteratorMap_.bucket_count();
		}
		
		void reserve(const size_t count) {
			iteratorMap_.reserve(count);
		}
		
	private:
		class SetArray {
		public:
			SetArray()
			: ptr_(static_cast<Value*>(malloc(sizeof(Value) * IndividualValueArraySize))) { }
			
			~SetArray() {
				free(ptr_);
			}
			
			SetArray(SetArray&& other)
			: ptr_(nullptr) {
				this->swap(other);
			}
			
			SetArray& operator=(SetArray&& other) {
				SetArray tmp(std::move(other));
				this->swap(tmp);
				return *this;
			}
			
			void swap(SetArray& other) {
				std::swap(ptr_, other.ptr_);
			}
			
			Value* data() {
				return ptr_;
			}
			
			const Value* data() const {
				return ptr_;
			}
			
			Value& operator[](const size_t index) {
				return ptr_[index];
			}
			
			const Value& operator[](const size_t index) const {
				return ptr_[index];
			}
			
		private:
			SetArray(const SetArray&) = delete;
			SetArray& operator=(const SetArray&) = delete;
			
			Value* ptr_;
			
		};
		
		iterator allocateValue(Value value) {
			const size_t valueId = nextValue_++;
			if ((valueId % IndividualValueArraySize) == 0) {
				arrays_.push_back(SetArray());
			}
			const size_t valueIndex = (valueId % IndividualValueArraySize);
			new(&(arrays_.back()[valueIndex])) Value(std::move(value));
			return iterator(arrays_, valueId);
		}
		
		struct hashTypePtr {
			inline std::size_t operator()(const Value* const value) const {
				std::hash<Value> hashFn;
				return hashFn(*value);
			}
		};
		
		struct isEqualTypePtr {
			inline bool operator()(const Value* const a, const Value* const b) const {
				return *a == *b;
			}
		};
		
		ValueArrays arrays_;
		size_t nextValue_;
		std::unordered_map<const Value*, iterator, hashTypePtr, isEqualTypePtr> iteratorMap_;
		
	};
	
}

#endif
