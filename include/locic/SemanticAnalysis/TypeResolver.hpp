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
			resolveType(AST::Node<AST::TypeDecl>& typeDecl);
			
			SEM::Alias*
			getTemplateVarTypeAlias(const AST::Node<AST::TypeDecl>& type);
			
			SEM::Predicate
			getTemplateVarTypePredicate(const AST::Node<AST::TypeDecl>& type,
			                            const SEM::TemplateVar& templateVar);
			
			const SEM::Type*
			resolveTemplateVarType(AST::Node<AST::TypeDecl>& typeDecl);
			
		private:
			const SEM::Type*
			convertType(AST::Node<AST::TypeDecl>& type);
			
			Context& context_;
			
		};
		
	}
	
}

#endif
