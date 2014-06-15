#ifndef LOCIC_SEM_TYPEALIAS_HPP
#define LOCIC_SEM_TYPEALIAS_HPP

#include <string>

#include <locic/Name.hpp>

namespace locic {

	namespace SEM {
	
		class Type;
		
		class TypeAlias {
			public:
				TypeAlias(const Name& pName);
				
				const Name& name() const;
				
				Type* value() const;
				
				void setValue(Type* pValue);
				
				std::string toString() const;
				
			private:
				Name name_;
				Type* value_;
				
		};
		
	}
	
}

#endif
