#include <locic/AST.hpp>
#include <locic/Parser/SymbolBuilder.hpp>
#include <locic/Parser/SymbolParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/ValueParser.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenReader;
		
		SymbolParser::SymbolParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		SymbolParser::~SymbolParser() { }
		
		AST::Node<AST::Symbol> SymbolParser::parseSymbol() {
			const auto start = reader_.position();
			
			const auto firstSymbolElement = parseSymbolElement();
			
			auto symbol = AST::Symbol::Relative() + firstSymbolElement;
			
			while (true) {
				if (reader_.peek().kind() != Token::DOUBLE_COLON) {
					break;
				}
				
				reader_.consume();
				
				const auto symbolElement = parseSymbolElement();
				symbol = symbol + symbolElement;
			}
			
			return builder_.makeSymbolNode(symbol, start);
		}
		
		AST::Node<AST::SymbolElement> SymbolParser::parseSymbolElement() {
			const auto start = reader_.position();
			
			const auto token = reader_.get();
			assert(token.kind() == Token::TYPENAME ||
			       token.kind() == Token::NAME);
			
			if (token.kind() == Token::TYPENAME) {
				return builder_.makeTypenameSymbolElement(start);
			}
			
			const auto name = token.name();
			const auto templateArguments = parseSymbolTemplateArgumentList();
			return builder_.makeSymbolElement(name, templateArguments, start);
		}
		
		AST::Node<AST::ValueList> SymbolParser::parseSymbolTemplateArgumentList() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::LTRIBRACKET) {
				return builder_.makeValueList({}, start);
			}
			
			reader_.consume();
			
			AST::ValueList valueList;
			
			while (true) {
				const auto value = ValueParser(reader_).parseValue();
				valueList.push_back(value);
				
				if (reader_.peek().kind() != Token::COMMA) {
					break;
				}
				
				reader_.consume();
			}
			
			reader_.expect(Token::RTRIBRACKET);
			
			return builder_.makeValueList(valueList, start);
		}
		
	}
	
}
