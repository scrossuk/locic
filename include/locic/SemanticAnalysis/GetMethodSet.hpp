#ifndef LOCIC_SEMANTICANALYSIS_GETMETHODSET_HPP
#define LOCIC_SEMANTICANALYSIS_GETMETHODSET_HPP

namespace locic {
	
	namespace AST {
		
		class MethodSet;
		class Predicate;
		class TemplateVar;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		const AST::MethodSet*
		getMethodSetForRequiresPredicate(AST::TemplateVar* templateVar, const AST::Predicate& requiresPredicate);
		
		const AST::MethodSet*
		getMethodSetForObjectType(Context& context, const AST::Type* objectType);
		
		const AST::MethodSet*
		getTypeMethodSet(Context& context, const AST::Type* type);
		
		const AST::MethodSet*
		intersectMethodSets(const AST::MethodSet* setA, const AST::MethodSet* setB);
		
		const AST::MethodSet*
		unionMethodSets(const AST::MethodSet* setA, const AST::MethodSet* setB);
		
	}
	
}

#endif
