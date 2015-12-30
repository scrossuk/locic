#ifndef LOCIC_PARSER_TEMPLATEPARSER_HPP
#define LOCIC_PARSER_TEMPLATEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/TemplateBuilder.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TemplateInfo;
		class TokenReader;
		
		class TemplateParser {
		public:
			TemplateParser(TokenReader& reader);
			~TemplateParser();
			
			TemplateInfo parseTemplate();
			
			AST::Node<AST::TemplateTypeVarList> parseTemplateVarList();
			
			AST::Node<AST::TemplateTypeVar> parseTemplateVar();
			
		private:
			TokenReader& reader_;
			TemplateBuilder builder_;
			
		};
		
	}
	
}

#endif