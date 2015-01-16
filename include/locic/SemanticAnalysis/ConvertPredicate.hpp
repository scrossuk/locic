#ifndef LOCIC_SEMANTICANALYSIS_CONVERTPREDICATE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTPREDICATE_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		SEM::Predicate ConvertConstSpecifier(Context& context, const AST::Node<AST::ConstSpecifier>& astConstSpecifierNode);
		
		SEM::Predicate ConvertRequireSpecifier(Context& context, const AST::Node<AST::RequireSpecifier>& astRequireSpecifierNode);
		
		bool evaluatePredicate(Context& context, const SEM::Predicate& predicate, const SEM::TemplateVarMap& variableAssignments);
		
		// TODO: take a value rather than a reference.
		SEM::Predicate simplifyPredicate(const SEM::Predicate& predicate);
		
	}
	
}

#endif
