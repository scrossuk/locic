#ifndef LOCIC_CODE_VAR_HPP
#define LOCIC_CODE_VAR_HPP

#include <Locic/CODE/Type.hpp>

namespace Locic {

	namespace CODE {
	
		class Var{
			public:
				inline Var(Type * type)
					: type_(type){ }
				
				inline Type * getType(){
					return type_;
				}
			
			private:
				Type * type_;
			
		};
	}
	
}

#endif
