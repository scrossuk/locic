#include <locic/AST/Function.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SearchResult SearchResult::None() {
			return SearchResult(NONE);
		}
		
		SearchResult SearchResult::Alias(AST::Alias& alias) {
			SearchResult element(ALIAS);
			element.data_.alias = &alias;
			return element;
		}
		
		SearchResult SearchResult::Function(AST::Function& function) {
			SearchResult element(FUNCTION);
			element.data_.function = &function;
			return element;
		}
		
		SearchResult SearchResult::TemplateVar(AST::TemplateVar& templateVar) {
			SearchResult element(TEMPLATEVAR);
			element.data_.templateVar = &templateVar;
			return element;
		}
		
		SearchResult SearchResult::TypeInstance(SEM::TypeInstance& typeInstance) {
			SearchResult element(TYPEINSTANCE);
			element.data_.typeInstance = &typeInstance;
			return element;
		}
		
		SearchResult SearchResult::Var(AST::Var& var) {
			SearchResult element(VAR);
			element.data_.var = &var;
			return element;
		}
		
		SearchResult::Kind SearchResult::kind() const {
			return kind_;
		}
		
		bool SearchResult::isNone() const {
			return kind() == NONE;
		}
		
		bool SearchResult::isAlias() const {
			return kind() == ALIAS;
		}
		
		bool SearchResult::isFunction() const {
			return kind() == FUNCTION;
		}
		
		bool SearchResult::isTemplateVar() const {
			return kind() == TEMPLATEVAR;
		}
		
		bool SearchResult::isTypeInstance() const {
			return kind() == TYPEINSTANCE;
		}
		
		bool SearchResult::isVar() const {
			return kind() == VAR;
		}
		
		AST::Alias& SearchResult::alias() const {
			assert(isAlias());
			return *(data_.alias);
		}
		
		AST::Function& SearchResult::function() const {
			assert(isFunction());
			return *(data_.function);
		}
		
		AST::TemplateVar& SearchResult::templateVar() const {
			assert(isTemplateVar());
			return *(data_.templateVar);
		}
		
		SEM::TypeInstance& SearchResult::typeInstance() const {
			assert(isTypeInstance());
			return *(data_.typeInstance);
		}
		
		AST::Var& SearchResult::var() const {
			assert(isVar());
			return *(data_.var);
		}
		
		SearchResult::SearchResult(Kind pKind)
			: kind_(pKind) {
				data_.ptr = nullptr;
			}
		
	}
	
}

