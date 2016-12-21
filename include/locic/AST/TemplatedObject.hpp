#ifndef LOCIC_AST_TEMPLATEDOBJECT_HPP
#define LOCIC_AST_TEMPLATEDOBJECT_HPP

#include <vector>

#include <locic/AST/TemplateVarArray.hpp>

namespace locic {
	
	class Name;
	
	namespace AST {
		
		class Predicate;
		
		class TemplatedObject {
			public:
				virtual const Name& fullName() const = 0;
				
				virtual TemplateVarArray& templateVariables() = 0;
				
				virtual const TemplateVarArray& templateVariables() const = 0;
				
				virtual const Predicate& requiresPredicate() const = 0;
				
				virtual const Predicate& noexceptPredicate() const = 0;
				
			protected:
				~TemplatedObject() { }
				
		};
		
	}
	
}

#endif
