#ifndef LOCIC_SEM_TYPEALIAS_HPP
#define LOCIC_SEM_TYPEALIAS_HPP

#include <string>

#include <locic/Name.hpp>

namespace locic {

	namespace SEM {
	
		class Context;
		class Type;
		
		class TypeAlias {
			public:
				TypeAlias(Context& pContext, const Name& pName);
				
				Context& context() const;
				
				const Name& name() const;
				
				Type* value() const;
				
				void setValue(Type* pValue);
				
				std::string toString() const;
				
			private:
				Context& context_;
				Name name_;
				Type* value_;
				
		};
		
	}
	
}

#endif
