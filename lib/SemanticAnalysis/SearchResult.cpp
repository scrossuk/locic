#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SearchResult SearchResult::None() {
			return SearchResult(NONE);
		}
		
		SearchResult SearchResult::Function(SEM::Function* function) {
			SearchResult element(FUNCTION);
			element.data_.function = function;
			return element;
		}
		
		SearchResult SearchResult::TemplateVar(SEM::TemplateVar* templateVar) {
			SearchResult element(TEMPLATEVAR);
			element.data_.templateVar = templateVar;
			return element;
		}
		
		SearchResult SearchResult::TypeAlias(SEM::TypeAlias* typeAlias) {
			SearchResult element(TYPEALIAS);
			element.data_.typeAlias = typeAlias;
			return element;
		}
		
		SearchResult SearchResult::TypeInstance(SEM::TypeInstance* typeInstance) {
			SearchResult element(TYPEINSTANCE);
			element.data_.typeInstance = typeInstance;
			return element;
		}
		
		SearchResult SearchResult::Var(SEM::Var* var) {
			SearchResult element(VAR);
			element.data_.var = var;
			return element;
		}
		
		SearchResult::Kind SearchResult::kind() const {
			return kind_;
		}
		
		bool SearchResult::isNone() const {
			return kind() == NONE;
		}
		
		bool SearchResult::isFunction() const {
			return kind() == FUNCTION;
		}
		
		bool SearchResult::isTemplateVar() const {
			return kind() == TEMPLATEVAR;
		}
		
		bool SearchResult::isTypeAlias() const {
			return kind() == TYPEALIAS;
		}
		
		bool SearchResult::isTypeInstance() const {
			return kind() == TYPEINSTANCE;
		}
		
		bool SearchResult::isVar() const {
			return kind() == VAR;
		}
		
		SEM::Function* SearchResult::function() const {
			assert(isFunction());
			return data_.function;
		}
		
		SEM::TemplateVar* SearchResult::templateVar() const {
			assert(isTemplateVar());
			return data_.templateVar;
		}
		
		SEM::TypeAlias* SearchResult::typeAlias() const {
			assert(isTypeAlias());
			return data_.typeAlias;
		}
		
		SEM::TypeInstance* SearchResult::typeInstance() const {
			assert(isTypeInstance());
			return data_.typeInstance;
		}
		
		SEM::Var* SearchResult::var() const {
			assert(isVar());
			return data_.var;
		}
		
		SearchResult::SearchResult(Kind pKind)
			: kind_(pKind) {
				data_.ptr = nullptr;
			}
		
	}
	
}

