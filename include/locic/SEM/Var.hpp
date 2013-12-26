#ifndef LOCIC_SEM_VAR_HPP
#define LOCIC_SEM_VAR_HPP

#include <stdint.h>
#include <string>

#include <locic/SEM/Object.hpp>

namespace locic {

	namespace SEM {
	
		class Type;
		
		class Var: public Object {
			public:
				enum Kind {
					LOCAL,
					PARAM,
					MEMBER
				};
				
				static inline Var * Local(Type * type){
					return new Var(LOCAL, type);
				}
				
				static inline Var * Param(Type * type){
					return new Var(PARAM, type);
				}
				
				static inline Var * Member(Type * type){
					return new Var(MEMBER, type);
				}
				
				inline ObjectKind objectKind() const {
					return OBJECT_VARIABLE;
				}
				
				inline Kind kind() const {
					return kind_;
				}
				
				inline Type* type() const {
					return type_;
				}
				
				std::string toString() const;
				
			private:
				inline Var(Kind k, Type* t)
					: kind_(k), type_(t) {
					assert(type_ != NULL);
				}
				
				Kind kind_;
				Type* type_;
				
		};
		
	}
	
}

#endif
