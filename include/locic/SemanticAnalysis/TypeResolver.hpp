#ifndef LOCIC_SEMANTICANALYSIS_TYPERESOLVER_HPP
#define LOCIC_SEMANTICANALYSIS_TYPERESOLVER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class String;
	
	namespace AST {
		
		class Alias;
		class TemplateVar;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		class TypeResolver {
		public:
			TypeResolver(Context& context);
			
			const AST::Type*
			resolveObjectType(const AST::Node<AST::Symbol>& symbol);
			
			const AST::Type*
			resolveIntegerType(AST::TypeDecl::SignedModifier signedModifier,
			                   const String& nameString);
			
			const AST::Type*
			resolveFloatType(const String& nameString);
			
			const AST::Type*
			resolveType(AST::Node<AST::TypeDecl>& typeDecl);
			
			AST::Alias*
			getTemplateVarTypeAlias(const AST::Node<AST::TypeDecl>& type);
			
			AST::Predicate
			getTemplateVarTypePredicate(const AST::Node<AST::TypeDecl>& type,
			                            const AST::TemplateVar& templateVar);
			
			const AST::Type*
			resolveTemplateVarType(AST::Node<AST::TypeDecl>& typeDecl);
			
		private:
			const AST::Type*
			convertType(AST::Node<AST::TypeDecl>& type);
			
			Context& context_;
			
		};
		
	}
	
}

#endif
