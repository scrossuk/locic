#ifndef LOCIC_SEMANTICANALYSIS_SATISFYCHECKER_HPP
#define LOCIC_SEMANTICANALYSIS_SATISFYCHECKER_HPP

#include <vector>

#include <locic/AST/ValueArray.hpp>

#include <locic/Frontend/ResultOrDiag.hpp>

#include <locic/Support/Array.hpp>

namespace locic {
	
	class OptionalDiag;
	class String;
	
	namespace AST {
		
		class MethodSet;
		class MethodSetElement;
		class Predicate;
		class TemplateVarMap;
		class Type;
		class Value;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		class Unifier;
		
		class SatisfyChecker {
		public:
			using Stack = Array<std::pair<const AST::Type*, const AST::Type*>, 10>;
			
			SatisfyChecker(Context& context, Unifier& unifier);
			
			ResultOrDiag<const AST::Type*>
			satisfies(const AST::Type* checkType,
			          const AST::Type* requireType);
			
			ResultOrDiag<const AST::Type*>
			typeSatisfies(const AST::Type* checkType,
			              const AST::Type* requireType);
			
			OptionalDiag
			methodSetElementTypeCast(const AST::Type* sourceType,
			                         const AST::Type* destType);
			
			AST::TemplateVarMap
			generateSatisfyTemplateVarMap(const AST::MethodSetElement& checkElement,
			                              const AST::MethodSetElement& requireElement);
			
			OptionalDiag
			methodSatisfies(const AST::Predicate& checkSelfConst,
			                const AST::Predicate& requireSelfConst,
			                const String& functionName,
			                const AST::MethodSetElement& checkFunctionElement,
			                const AST::MethodSetElement& requireFunctionElement);
		
			OptionalDiag
			methodSetSatisfies(const AST::MethodSet* checkSet,
			                   const AST::MethodSet* requireSet);
			
			AST::Predicate reducePredicate(AST::Predicate predicate);
			
		private:
			Context& context_;
			Unifier& unifier_;
			Stack satisfyCheckStack_;
			
		};
		
	}
	
}

#endif
