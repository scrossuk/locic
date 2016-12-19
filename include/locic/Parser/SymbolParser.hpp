#ifndef LOCIC_PARSER_SYMBOLPARSER_HPP
#define LOCIC_PARSER_SYMBOLPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/SymbolBuilder.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenReader;
		
		class SymbolParser {
		public:
			enum Context {
				GENERIC,
				
				// If we're in the middle of parsing a type
				// then '<' always means the beginning of
				// template arguments.
				IN_TYPE
			};
			
			SymbolParser(TokenReader& reader);
			~SymbolParser();
			
			AST::Node<AST::Symbol> parseSymbol(Context context = GENERIC);
			
			AST::Node<AST::SymbolElement> parseSymbolElement(Context context);
			
			AST::Node<AST::ValueDeclList>
			parseSymbolTemplateArgumentList(Context context = GENERIC);
			
			bool isNowAtTemplateArgumentList(Context context);
			
			bool isValidTokenAfterTemplateArguments(size_t offset);
			
		private:
			TokenReader& reader_;
			SymbolBuilder builder_;
			
		};
		
	}
	
}

#endif
