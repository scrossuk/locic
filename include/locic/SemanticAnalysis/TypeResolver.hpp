#ifndef LOCIC_SEMANTICANALYSIS_TYPERESOLVER_HPP
#define LOCIC_SEMANTICANALYSIS_TYPERESOLVER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class String;
	
	namespace SEM {
		
		class TemplateVar;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		class TypeResolver {
		public:
			TypeResolver(Context& context);
			
			const SEM::Type*
			resolveObjectType(const AST::Node<AST::Symbol>& symbol);
			
			const SEM::Type*
			resolveIntegerType(AST::TypeDecl::SignedModifier signedModifier,
			                   const String& nameString);
			
			const SEM::Type*
			resolveFloatType(const String& nameString);
			
			const SEM::Type*
			resolveType(const AST::Node<AST::TypeDecl>& typeDecl);
			
			SEM::Alias*
			getTemplateVarTypeAlias(const AST::Node<AST::TypeDecl>& type);
			
			SEM::Predicate
			getTemplateVarTypePredicate(const AST::Node<AST::TypeDecl>& type,
			                            const SEM::TemplateVar& templateVar);
			
			const SEM::Type*
			resolveTemplateVarType(const AST::Node<AST::TypeDecl>& typeDecl);
			
		private:
			Context& context_;
			
		};
		
	}
	
}

#endif
