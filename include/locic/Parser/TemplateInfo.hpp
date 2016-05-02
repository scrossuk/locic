#ifndef LOCIC_PARSER_TEMPLATEINFO_HPP
#define LOCIC_PARSER_TEMPLATEINFO_HPP

#include <locic/AST.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace Parser {
		
		class TemplateInfo {
		public:
			TemplateInfo();
			~TemplateInfo();
			
			TemplateInfo(TemplateInfo&&) = default;
			TemplateInfo& operator=(TemplateInfo&&) = default;
			
			const AST::Node<AST::TemplateVarList>& templateVariables() const;
			const AST::Node<AST::RequireSpecifier>& requireSpecifier() const;
			const AST::Node<AST::RequireSpecifier>& moveSpecifier() const;
			const AST::Node<AST::StringList>& noTagSet() const;
			
			AST::Node<AST::TemplateVarList> extractTemplateVariables();
			AST::Node<AST::RequireSpecifier> extractRequireSpecifier();
			AST::Node<AST::RequireSpecifier> extractMoveSpecifier();
			AST::Node<AST::StringList> extractNoTagSet();
			
			bool hasRequireSpecifier() const;
			bool hasMoveSpecifier() const;
			bool hasNoTagSet() const;
			
			void setTemplateVariables(AST::Node<AST::TemplateVarList> templateVariables);
			void setRequireSpecifier(AST::Node<AST::RequireSpecifier> specifier);
			void setMoveSpecifier(AST::Node<AST::RequireSpecifier> specifier);
			void setNoTagSet(AST::Node<AST::StringList> noTagSet);
			
		private:
			Optional<AST::Node<AST::TemplateVarList>> templateVariables_;
			Optional<AST::Node<AST::RequireSpecifier>> requireSpecifier_;
			Optional<AST::Node<AST::RequireSpecifier>> moveSpecifier_;
			Optional<AST::Node<AST::StringList>> noTagSet_;
			
		};
		
	}
	
}

#endif