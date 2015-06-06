#ifndef LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP
#define LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP

namespace locic {
	
	namespace SEM {
		
		class Alias;
		class Function;
		class TemplateVar;
		class TypeInstance;
		class Var;
		
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
				
				static SearchResult Alias(SEM::Alias* alias);
				
				static SearchResult Function(SEM::Function* function);
				
				static SearchResult TemplateVar(SEM::TemplateVar* templateVar);
				
				static SearchResult TypeInstance(SEM::TypeInstance* typeInstance);
				
				static SearchResult Var(SEM::Var* var);
				
				Kind kind() const;
				
				bool isNone() const;
				bool isAlias() const;
				bool isFunction() const;
				bool isTemplateVar() const;
				bool isTypeInstance() const;
				bool isVar() const;
				
				SEM::Alias* alias() const;
				SEM::Function* function() const;
				SEM::TemplateVar* templateVar() const;
				SEM::TypeInstance* typeInstance() const;
				SEM::Var* var() const;
				
			private:
				SearchResult(Kind pKind);
				
				Kind kind_;
				
				union {
					void* ptr;
					SEM::Alias* alias;
					SEM::Function* function;
					SEM::TemplateVar* templateVar;
					SEM::TypeInstance* typeInstance;
					SEM::Var* var;
				} data_;
				
		};
		
	}
	
}

#endif
