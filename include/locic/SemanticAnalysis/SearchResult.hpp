#ifndef LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP
#define LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP

namespace locic {
	
	namespace AST {
		
		class Alias;
		class Function;
		class TemplateVar;
		class Var;
		
	}
	
	namespace SEM {
		
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
				
				static SearchResult Alias(AST::Alias& alias);
				
				static SearchResult Function(AST::Function& function);
				
				static SearchResult TemplateVar(AST::TemplateVar& templateVar);
				
				static SearchResult TypeInstance(SEM::TypeInstance& typeInstance);
				
				static SearchResult Var(AST::Var& var);
				
				Kind kind() const;
				
				bool isNone() const;
				bool isAlias() const;
				bool isFunction() const;
				bool isTemplateVar() const;
				bool isTypeInstance() const;
				bool isVar() const;
				
				AST::Alias& alias() const;
				AST::Function& function() const;
				AST::TemplateVar& templateVar() const;
				SEM::TypeInstance& typeInstance() const;
				AST::Var& var() const;
				
			private:
				SearchResult(Kind pKind);
				
				Kind kind_;
				
				union {
					void* ptr;
					AST::Alias* alias;
					AST::Function* function;
					AST::TemplateVar* templateVar;
					SEM::TypeInstance* typeInstance;
					AST::Var* var;
				} data_;
				
		};
		
	}
	
}

#endif
