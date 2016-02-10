#ifndef LOCIC_SUPPORT_OPTIONAL_HPP
#define LOCIC_SUPPORT_OPTIONAL_HPP

#include <cassert>
#include <cstring>
#include <type_traits>

namespace locic {
	
	// As a slightly nasty trick, represent NoneType as a pointer to member,
	// since this encodes the NoneType-specific helper struct into the type
	// while still being a literal type.
	namespace Helper { struct NoneHelper{}; }
	
	using NoneType = int Helper::NoneHelper::*;
	
	constexpr NoneType None = static_cast<NoneType>(0);
	
	template <typename Value, bool = std::is_copy_constructible<Value>::value>
	class Optional {
		public:
			Optional()
			: hasValue_(false) { }
			
			Optional(NoneType)
			: hasValue_(false) { }
			
			Optional(const Optional& other)
			: hasValue_(other) {
				if (hasValue_) {
					new (ptr()) Value(*other);
				}
			}
			
			Optional(Optional&& other)
			: hasValue_(false) {
				memset(&data_, 0, sizeof(data_));
				swap(other);
			}
			
			explicit Optional(Value argValue)
			: hasValue_(true) {
				new (ptr()) Value(std::move(argValue));
			}
			
			~Optional() {
				if (hasValue_) {
					// Call destructor directly.
					ptr()->~Value();
				}
			}
			
			Optional& operator=(const Optional& other) {
				Optional tmp(other);
				swap(tmp);
				return *this;
			}
			
			Optional& operator=(Optional&& other) {
				Optional tmp(std::move(other));
				swap(tmp);
				return *this;
			}
			
			Optional& operator=(Value other) {
				Optional tmp(std::move(other));
				swap(tmp);
				return *this;
			}
			
			void swap(Optional& other) {
				if (*this) {
					if (other) {
						// Both optionals set; swap values.
						std::swap(*(ptr()), *other);
					} else {
						// Other optional isn't set; move-construct
						// our value into it.
						new (other.ptr()) Value(std::move(*(ptr())));
						
						// Call destructor on value moved from.
						ptr()->~Value();
					}
				} else {
					if (other) {
						// Our optional isn't set; move-construct
						// other value into us.
						new (ptr()) Value(std::move(*(other.ptr())));
						
						// Call destructor on value moved from.
						other.ptr()->~Value();
					} else {
						// Nothing to move; both optionals aren't set.
					}
				}
				std::swap(hasValue_, other.hasValue_);
			}
			
			operator bool() const {
				return hasValue_;
			}
			
			Value* operator->() {
				assert(*this);
				return ptr();
			}
			
			const Value* operator->() const {
				assert(*this);
				return ptr();
			}
			
			Value& operator*() {
				assert(*this);
				return *(ptr());
			}
			
			const Value& operator*() const {
				assert(*this);
				return *(ptr());
			}
			
			Value& value() {
				assert(*this);
				return *(ptr());
			}
			
			const Value& value() const {
				assert(*this);
				return *(ptr());
			}
			
		private:
			Value* ptr() {
				return reinterpret_cast<Value*>(&data_);
			}
			
			const Value* ptr() const {
				return reinterpret_cast<const Value*>(&data_);
			}
			
			bool hasValue_;
			
			typedef typename std::aligned_storage<sizeof(Value), alignof(Value)>::type Data;
			Data data_;
		
	};
	
	template <typename Value>
	class Optional<Value, false>: public Optional<Value, true> {
		public:
			template <class... Args>
			Optional(Args... t) : Optional<Value, true>(std::forward<Args>(t)...) {}
			
			Optional(const Optional&) = delete;
			Optional& operator=(const Optional&) = delete;
			
 			Optional(Optional&&) = default;
 			Optional& operator=(Optional&&) = default;
			
	};
	
	template <typename Value>
	Optional<Value> make_optional(Value value){
		return Optional<Value>(std::move(value));
	}

}

#endif
