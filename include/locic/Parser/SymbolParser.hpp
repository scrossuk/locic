#ifndef LOCIC_PARSER_SYMBOLPARSER_HPP
#define LOCIC_PARSER_SYMBOLPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/SymbolBuilder.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenReader;
		
		class SymbolParser {
		public:
			SymbolParser(TokenReader& reader);
			~SymbolParser();
			
			AST::Node<AST::Symbol> parseSymbol();
			
			AST::Node<AST::SymbolElement> parseSymbolElement();
			
			AST::Node<AST::ValueList> parseSymbolTemplateArgumentList();
			
		private:
			TokenReader& reader_;
			SymbolBuilder builder_;
			
		};
		
	}
	
}

#endif
