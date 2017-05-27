#ifndef LOCIC_SEMANTICANALYSIS_UNIFIER_HPP
#define LOCIC_SEMANTICANALYSIS_UNIFIER_HPP

#include <locic/AST/ValueArray.hpp>

#include <locic/Frontend/ResultOrDiag.hpp>

namespace locic {
	
	namespace AST {
		
		class Predicate;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Unifier {
		public:
			Unifier();
			
			ResultOrDiag<const AST::Type*>
			unifyTypes(const AST::Type* first,
			           const AST::Type* second);
			
			ResultOrDiag<const AST::Type*>
			unifyNonConstTypes(const AST::Type* first,
			                   const AST::Type* second);
			
			ResultOrDiag<AST::ValueArray>
			unifyTemplateArgs(const AST::ValueArray& first,
			                  const AST::ValueArray& second);
			
			ResultOrDiag<const AST::Predicate*>
			unifyConstPredicates(const AST::Predicate& first,
			                     const AST::Predicate& second);
			
		private:
			// Currently no members.
			
		};
		
	}
	
}

#endif
