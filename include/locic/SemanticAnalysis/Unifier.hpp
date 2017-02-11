#ifndef LOCIC_SEMANTICANALYSIS_UNIFIER_HPP
#define LOCIC_SEMANTICANALYSIS_UNIFIER_HPP

#include <locic/AST/ValueArray.hpp>

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class Predicate;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Unifier {
		public:
			Unifier();
			
			OptionalDiag
			unifyTypes(const AST::Type* first,
			           const AST::Type* second);
			
			OptionalDiag
			unifyTemplateArgs(const AST::ValueArray& first,
			                  const AST::ValueArray& second);
			
			OptionalDiag
			unifyConstPredicates(const AST::Predicate& first,
			                     const AST::Predicate& second);
			
		private:
			// Currently no members.
			
		};
		
	}
	
}

#endif
