#ifndef LOCIC_SEM_VAR_HPP
#define LOCIC_SEM_VAR_HPP

#include <stdint.h>
#include <string>

namespace Locic {

	namespace SEM {
	
		class Type;
		class TypeInstance;
		
		class Var {
			public:
				enum Kind {
					LOCAL,
					PARAM,
					MEMBER
				};
				
				inline Var(Kind k, size_t i, Type* t, TypeInstance* p = NULL)
					: kind_(k), id_(i), type_(t), parent_(p) {
					assert(type_ != NULL);
				}
				
				inline Kind kind() const {
					return kind_;
				}
				
				inline size_t id() const {
					return id_;
				}
				
				inline Type* type() const {
					return type_;
				}
				
				inline bool hasParentType() const {
					return parent_ != NULL;
				}
				
				inline TypeInstance* getParentType() const {
					assert(hasParentType());
					return parent_;
				}
				
				std::string toString() const;
				
			private:
				Kind kind_;
				size_t id_;
				Type* type_;
				TypeInstance* parent_;
				
		};
		
	}
	
}

#endif
