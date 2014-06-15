#ifndef LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP
#define LOCIC_SEMANTICANALYSIS_SEARCHRESULT_HPP

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class SearchResult {
			public:
				enum Kind {
					NONE,
					FUNCTION,
					TEMPLATEVAR,
					TYPEALIAS,
					TYPEINSTANCE,
					VAR
				};
				
				static SearchResult None();
				
				static SearchResult Function(SEM::Function* function);
				
				static SearchResult TemplateVar(SEM::TemplateVar* templateVar);
				
				static SearchResult TypeAlias(SEM::TypeAlias* typeAlias);
				
				static SearchResult TypeInstance(SEM::TypeInstance* typeInstance);
				
				static SearchResult Var(SEM::Var* var);
				
				Kind kind() const;
				
				bool isNone() const;
				bool isFunction() const;
				bool isTemplateVar() const;
				bool isTypeAlias() const;
				bool isTypeInstance() const;
				bool isVar() const;
				
				SEM::Function* function() const;
				SEM::TemplateVar* templateVar() const;
				SEM::TypeAlias* typeAlias() const;
				SEM::TypeInstance* typeInstance() const;
				SEM::Var* var() const;
				
			private:
				SearchResult(Kind pKind);
				
				Kind kind_;
				
				union {
					void* ptr;
					SEM::Function* function;
					SEM::TemplateVar* templateVar;
					SEM::TypeAlias* typeAlias;
					SEM::TypeInstance* typeInstance;
					SEM::Var* var;
				} data_;
				
		};
		
	}
	
}

#endif
