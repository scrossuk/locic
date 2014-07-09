#ifndef LOCIC_SEM_CONTEXT_HPP
#define LOCIC_SEM_CONTEXT_HPP

#include <set>

#include <locic/SEM/Type.hpp>

namespace locic {

	namespace SEM {
	
		class Context {
			public:
				Context();
				
				Type* getType(Type type);
				
			private:
				std::set<Type> types_;
				
		};
		
	}
	
}

#endif
