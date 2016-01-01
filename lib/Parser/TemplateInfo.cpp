#include <locic/AST.hpp>
#include <locic/Parser/TemplateInfo.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace Parser {
		
		TemplateInfo::TemplateInfo() { }
		TemplateInfo::~TemplateInfo() { }
		
		AST::Node<AST::TemplateTypeVarList>
		TemplateInfo::templateVariables() const {
			return *templateVariables_;
		}
		
		AST::Node<AST::RequireSpecifier>
		TemplateInfo::requireSpecifier() const {
			return *requireSpecifier_;
		}
		
		AST::Node<AST::RequireSpecifier>
		TemplateInfo::moveSpecifier() const {
			return *moveSpecifier_;
		}
		
		AST::Node<AST::StringList>
		TemplateInfo::noTagSet() const {
			return *noTagSet_;
		}
		
		bool TemplateInfo::hasRequireSpecifier() const {
			return requireSpecifier_;
		}
		
		bool TemplateInfo::hasMoveSpecifier() const {
			return moveSpecifier_;
		}
		
		bool TemplateInfo::hasNoTagSet() const {
			return noTagSet_;
		}
		
		void TemplateInfo::setTemplateVariables(AST::Node<AST::TemplateTypeVarList> templateVariables) {
			templateVariables_ = make_optional(templateVariables);
		}
		
		void TemplateInfo::setRequireSpecifier(AST::Node<AST::RequireSpecifier> specifier) {
			requireSpecifier_ = make_optional(specifier);
		}
		
		void TemplateInfo::setMoveSpecifier(AST::Node<AST::RequireSpecifier> specifier) {
			moveSpecifier_ = make_optional(specifier);
		}
		
		void TemplateInfo::setNoTagSet(AST::Node<AST::StringList> noTagSet) {
			noTagSet_ = make_optional(noTagSet);
		}
		
		
	}
	
}
