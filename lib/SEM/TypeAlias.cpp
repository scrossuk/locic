#include <locic/Name.hpp>
#include <locic/String.hpp>

#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeAlias.hpp>

namespace locic {

	namespace SEM {
	
		TypeAlias::TypeAlias(const Name& pName)
			: name_(pName), value_(nullptr) { }
		
		const Name& TypeAlias::name() const {
			return name_;
		}
		
		Type* TypeAlias::value() const {
			return value_;
		}
		
		void TypeAlias::setValue(Type* pValue) {
			assert(value_ == nullptr);
			assert(pValue_ != nullptr);
			value_ = pValue;
		}
		
		std::string TypeAlias::toString() const {
			return makeString("TypeAlias(name = %s, value = %s)",
				name().toString().c_str(),
				value() != nullptr ? value()->toString().c_str() : "[NULL]");
		}
		
	}
	
}

