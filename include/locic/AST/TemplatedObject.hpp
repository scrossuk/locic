#ifndef LOCIC_AST_TEMPLATEDOBJECT_HPP
#define LOCIC_AST_TEMPLATEDOBJECT_HPP

#include <vector>

#include <locic/AST/TemplateVarArray.hpp>

namespace locic {
	
	class Name;
	
	namespace SEM {
		
		class Predicate;
		
	}
	
	namespace AST {
		
		class TemplatedObject {
			public:
				virtual const Name& fullName() const = 0;
				
				virtual TemplateVarArray& templateVariables() = 0;
				
				virtual const TemplateVarArray& templateVariables() const = 0;
				
				virtual const SEM::Predicate& requiresPredicate() const = 0;
				
				virtual const SEM::Predicate& noexceptPredicate() const = 0;
				
			protected:
				~TemplatedObject() { }
				
		};
		
	}
	
}

#endif
