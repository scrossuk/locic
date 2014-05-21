#ifndef LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP
#define LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class SearchResult {
			public:
				enum Kind {
					NONE,
					TYPEINSTANCE,
					FUNCTION,
					VAR,
					TEMPLATEVAR
				};
				
				static SearchResult None();
				
				static SearchResult TypeInstance(SEM::TypeInstance* typeInstance);
				
				static SearchResult Function(SEM::Function* function);
				
				static SearchResult Var(SEM::Var* var);
				
				static SearchResult TemplateVar(SEM::TemplateVar* templateVar);
				
				Kind kind() const;
				
				bool isNone() const;
				bool isTypeInstance() const;
				bool isFunction() const;
				bool isVar() const;
				bool isTemplateVar() const;
				
				SEM::TypeInstance* typeInstance() const;
				SEM::Function* function() const;
				SEM::Var* var() const;
				SEM::TemplateVar* templateVar() const;
				
			private:
				SearchResult(Kind pKind);
				
				Kind kind_;
				
				union {
					void* ptr;
					SEM::TypeInstance* typeInstance;
					SEM::Function* function;
					SEM::Var* var;
					SEM::TemplateVar* templateVar;
				} data_;
				
		};
		
	}
	
}

#endif
