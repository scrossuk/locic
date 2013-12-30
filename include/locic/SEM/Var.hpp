#ifndef LOCIC_SEM_VAR_HPP
#define LOCIC_SEM_VAR_HPP

#include <string>

namespace locic {

	namespace SEM {
	
		class Type;
		
		class Var {
			public:
				enum Kind {
					LOCAL,
					PARAM,
					MEMBER
				};
				
				static Var * Local(Type * type);
				
				static Var * Param(Type * type);
				
				static Var * Member(Type * type);
				
				Kind kind() const;
				
				Type* type() const;
				
				std::string toString() const;
				
			private:
				Var(Kind k, Type* t);
				
				Kind kind_;
				Type* type_;
				
		};
		
	}
	
}

#endif
