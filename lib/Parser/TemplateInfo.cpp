#include <locic/AST.hpp>
#include <locic/Parser/TemplateInfo.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace Parser {
		
		TemplateInfo::TemplateInfo() { }
		TemplateInfo::~TemplateInfo() { }
		
		const AST::Node<AST::TemplateVarList>&
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
		
		AST::Node<AST::TemplateVarList>
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
		
		bool TemplateInfo::hasRequireSpecifier() const {
			return requireSpecifier_;
		}
		
		bool TemplateInfo::hasMoveSpecifier() const {
			return moveSpecifier_;
		}
		
		void TemplateInfo::setTemplateVariables(AST::Node<AST::TemplateVarList> newTemplateVariables) {
			templateVariables_ = std::move(newTemplateVariables);
		}
		
		void TemplateInfo::setRequireSpecifier(AST::Node<AST::RequireSpecifier> specifier) {
			requireSpecifier_ = std::move(specifier);
		}
		
		void TemplateInfo::setMoveSpecifier(AST::Node<AST::RequireSpecifier> specifier) {
			moveSpecifier_ = std::move(specifier);
		}
		
		
	}
	
}
