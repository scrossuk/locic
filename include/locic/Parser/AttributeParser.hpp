#ifndef LOCIC_PARSER_ATTRIBUTEPARSER_HPP
#define LOCIC_PARSER_ATTRIBUTEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/AttributeBuilder.hpp>

namespace locic {
	
	class Name;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class AttributeParser {
		public:
			AttributeParser(TokenReader& reader);
			~AttributeParser();
			
			AST::Node<AST::ConstSpecifier>
			parseOptionalConstSpecifier();
			
			AST::Node<AST::RequireSpecifier>
			parseOptionalNoexceptSpecifier();
			
			AST::Node<AST::RequireSpecifier>
			parseOptionalRequireSpecifier();
			
		private:
			TokenReader& reader_;
			AttributeBuilder builder_;
			
		};
		
	}
	
}

#endif