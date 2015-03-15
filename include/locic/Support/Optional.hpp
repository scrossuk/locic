#ifndef LOCIC_SUPPORT_OPTIONAL_HPP
#define LOCIC_SUPPORT_OPTIONAL_HPP

#include <cassert>
#include <type_traits>

namespace locic{
	
	struct NoneType {
		NoneType() { }
	};
	
	static const NoneType None;
	
	template <typename Value>
	class Optional {
		public:
			Optional()
			: hasValue_(false) { }
			
			Optional(NoneType)
			: hasValue_(false) { }
			
			Optional(const Optional<Value>& other)
			: hasValue_(other) {
				if (hasValue_) {
					new (ptr()) Value(*other);
				}
			}
			
			Optional(Optional<Value>&& other)
			: hasValue_(false) {
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
			
			Optional<Value>& operator=(const Optional<Value>& other) {
				Optional<Value> tmp(other);
				swap(tmp);
				return *this;
			}
			
			Optional<Value>& operator=(Optional<Value>&& other) {
				Optional<Value> tmp(std::move(other));
				swap(tmp);
				return *this;
			}
			
			void swap(Optional<Value>& other) {
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
	Optional<Value> make_optional(Value value){
		return Optional<Value>(std::move(value));
	}

}

#endif
