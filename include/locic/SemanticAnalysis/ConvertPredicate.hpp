#ifndef LOCIC_SEMANTICANALYSIS_CONVERTPREDICATE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTPREDICATE_HPP

#include <locic/AST.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class Predicate;
		class TemplateVarMap;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		AST::Predicate ConvertPredicate(Context& context, const AST::Node<AST::PredicateDecl>& astPredicateNode);
		
		AST::Predicate ConvertConstSpecifier(Context& context, const AST::Node<AST::ConstSpecifier>& astConstSpecifierNode);
		
		AST::Predicate ConvertPredicateSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode,
			bool noneValue, bool noPredicateValue);
		
		inline AST::Predicate ConvertNoExceptSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode) {
			// Noexcept predicates are 'false' if not specified.
			const bool noneValue = false;
			
			// Noexcept predicates are 'true' if specified without a predicate.
			const bool noPredicateValue = true;
			
			return ConvertPredicateSpecifier(context, astRequireSpecifierNode, noneValue, noPredicateValue);
		}
		
		inline AST::Predicate ConvertRequireSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode) {
			// Require predicates are 'true' if not specified.
			const bool noneValue = true;
			
			// Not valid...
			const bool noPredicateValue = false;
			
			return ConvertPredicateSpecifier(context, astRequireSpecifierNode, noneValue, noPredicateValue);
		}
		
		OptionalDiag
		evaluatePredicate(Context& context, const AST::Predicate& predicate, const AST::TemplateVarMap& variableAssignments);
		
		AST::Predicate reducePredicate(Context& context, AST::Predicate predicate);
		
	}
	
}

#endif
