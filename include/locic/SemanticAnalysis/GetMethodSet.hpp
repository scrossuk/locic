#ifndef LOCIC_SEMANTICANALYSIS_GETMETHODSET_HPP
#define LOCIC_SEMANTICANALYSIS_GETMETHODSET_HPP

namespace locic {
	
	namespace AST {
		
		class TemplateVar;
		class Type;
		
	}
	
	namespace SEM {
		
		class Predicate;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		class MethodSet;
		
		const MethodSet* getMethodSetForRequiresPredicate(AST::TemplateVar* templateVar, const SEM::Predicate& requiresPredicate);
		
		const MethodSet* getMethodSetForObjectType(Context& context, const AST::Type* objectType);
		
		const MethodSet* getTypeMethodSet(Context& context, const AST::Type* type);
		
		const MethodSet* intersectMethodSets(const MethodSet* setA, const MethodSet* setB);
		
		const MethodSet* unionMethodSets(const MethodSet* setA, const MethodSet* setB);
		
	}
	
}

#endif
