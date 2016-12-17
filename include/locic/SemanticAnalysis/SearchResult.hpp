#ifndef LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP
#define LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP

namespace locic {
	
	namespace AST {
		
		class FunctionDecl;
		class Var;
		
	}
	
	namespace SEM {
		
		class Alias;
		class TemplateVar;
		class TypeInstance;
		
	}
	
	namespace SemanticAnalysis {
		
		class SearchResult {
			public:
				enum Kind {
					NONE,
					ALIAS,
					FUNCTION,
					TEMPLATEVAR,
					TYPEINSTANCE,
					VAR
				};
				
				static SearchResult None();
				
				static SearchResult Alias(SEM::Alias& alias);
				
				static SearchResult Function(AST::FunctionDecl& function);
				
				static SearchResult TemplateVar(SEM::TemplateVar& templateVar);
				
				static SearchResult TypeInstance(SEM::TypeInstance& typeInstance);
				
				static SearchResult Var(AST::Var& var);
				
				Kind kind() const;
				
				bool isNone() const;
				bool isAlias() const;
				bool isFunction() const;
				bool isTemplateVar() const;
				bool isTypeInstance() const;
				bool isVar() const;
				
				SEM::Alias& alias() const;
				AST::FunctionDecl& function() const;
				SEM::TemplateVar& templateVar() const;
				SEM::TypeInstance& typeInstance() const;
				AST::Var& var() const;
				
			private:
				SearchResult(Kind pKind);
				
				Kind kind_;
				
				union {
					void* ptr;
					SEM::Alias* alias;
					AST::FunctionDecl* function;
					SEM::TemplateVar* templateVar;
					SEM::TypeInstance* typeInstance;
					AST::Var* var;
				} data_;
				
		};
		
	}
	
}

#endif
