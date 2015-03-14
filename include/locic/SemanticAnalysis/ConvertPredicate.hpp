#ifndef LOCIC_SEMANTICANALYSIS_CONVERTPREDICATE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTPREDICATE_HPP

#include <locic/AST.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace SEM {
		
		class Predicate;
		class TemplateVarMap;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		SEM::Predicate ConvertConstSpecifier(Context& context, const AST::Node<AST::ConstSpecifier>& astConstSpecifierNode);
		
		SEM::Predicate ConvertRequireSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode);
		
		Optional<bool> evaluatePredicate(Context& context, const SEM::Predicate& predicate, const SEM::TemplateVarMap& variableAssignments);
		
		bool evaluatePredicateWithDefault(Context& context, const SEM::Predicate& predicate, const SEM::TemplateVarMap& variableAssignments, bool defaultValue);
		
		// TODO: take a value rather than a reference.
		SEM::Predicate simplifyPredicate(const SEM::Predicate& predicate);
		
	}
	
}

#endif
