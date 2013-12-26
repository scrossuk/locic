#ifndef LOCIC_OPTIONAL_HPP
#define LOCIC_OPTIONAL_HPP

#include <cassert>
#include <boost/optional.hpp>

namespace locic{

	template <typename Value>
	class Optional{
		public:
			Optional()
				: optional_(){ }
			
			explicit Optional(const Value& value)
				: optional_(value){ }
			
			bool hasValue() const{
				return (bool) optional_;
			}
			
			Value getValue() const{
				assert(hasValue());
				return optional_.get();
			}
			
		private:
			boost::optional<Value> optional_;
		
	};
	
	template <typename Value>
	Optional<Value> make_optional(const Value& value){
		return Optional<Value>(value);
	}

}

#endif
