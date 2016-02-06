#include <locic/AST.hpp>
#include <locic/Parser/TemplateInfo.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace Parser {
		
		TemplateInfo::TemplateInfo() { }
		TemplateInfo::~TemplateInfo() { }
		
		const AST::Node<AST::TemplateTypeVarList>&
		TemplateInfo::templateVariables() const {
			return *templateVariables_;
		}
		
		const AST::Node<AST::RequireSpecifier>&
		TemplateInfo::requireSpecifier() const {
			return *requireSpecifier_;
		}
		
		const AST::Node<AST::RequireSpecifier>&
		TemplateInfo::moveSpecifier() const {
			return *moveSpecifier_;
		}
		
		const AST::Node<AST::StringList>&
		TemplateInfo::noTagSet() const {
			return *noTagSet_;
		}
		
		AST::Node<AST::TemplateTypeVarList>
		TemplateInfo::extractTemplateVariables() {
			return std::move(*templateVariables_);
		}
		
		AST::Node<AST::RequireSpecifier>
		TemplateInfo::extractRequireSpecifier() {
			return std::move(*requireSpecifier_);
		}
		
		AST::Node<AST::RequireSpecifier>
		TemplateInfo::extractMoveSpecifier() {
			return std::move(*moveSpecifier_);
		}
		
		AST::Node<AST::StringList>
		TemplateInfo::extractNoTagSet() {
			return std::move(*noTagSet_);
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
		
		void TemplateInfo::setTemplateVariables(AST::Node<AST::TemplateTypeVarList> newTemplateVariables) {
			templateVariables_ = std::move(newTemplateVariables);
		}
		
		void TemplateInfo::setRequireSpecifier(AST::Node<AST::RequireSpecifier> specifier) {
			requireSpecifier_ = std::move(specifier);
		}
		
		void TemplateInfo::setMoveSpecifier(AST::Node<AST::RequireSpecifier> specifier) {
			moveSpecifier_ = std::move(specifier);
		}
		
		void TemplateInfo::setNoTagSet(AST::Node<AST::StringList> newNoTagSet) {
			noTagSet_ = std::move(newNoTagSet);
		}
		
		
	}
	
}
