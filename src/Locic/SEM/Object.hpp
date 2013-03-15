#ifndef LOCIC_SEM_OBJECT_HPP
#define LOCIC_SEM_OBJECT_HPP

namespace Locic {

	namespace SEM {
		
		enum ObjectKind {
			OBJECT_FUNCTION,
			OBJECT_NAMESPACE,
			OBJECT_SCOPE,
			OBJECT_STATEMENT,
			OBJECT_TEMPLATEVAR,
			OBJECT_TYPE,
			OBJECT_TYPEINSTANCE,
			OBJECT_VALUE,
			OBJECT_VARIABLE
		};
	
		class Object {
			public:
				inline virtual ~Object(){ }
				
				virtual ObjectKind objectKind() const = 0;
				
		};
		
	}
	
}

#endif
