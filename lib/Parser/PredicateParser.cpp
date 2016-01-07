#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/PredicateBuilder.hpp>
#include <locic/Parser/PredicateParser.hpp>
#include <locic/Parser/SymbolParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeParser.hpp>

namespace locic {
	
	namespace Parser {
		
		PredicateParser::PredicateParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		PredicateParser::~PredicateParser() { }
		
		AST::Node<AST::Predicate> PredicateParser::parsePredicate() {
			return parseBinaryPredicate();
		}
		
		AST::Node<AST::Predicate> PredicateParser::parseBinaryPredicate() {
			const auto start = reader_.position();
			
			auto predicate = parseAtomPredicate();
			
			while (true) {
				switch (reader_.peek().kind()) {
					case Token::AND: {
						reader_.consume();
						const auto rightPredicate = parseAtomPredicate();
						predicate = builder_.makeAndPredicate(predicate,
						                                      rightPredicate,
						                                      start);
						break;
					}
					case Token::OR: {
						reader_.consume();
						const auto rightPredicate = parseAtomPredicate();
						predicate = builder_.makeOrPredicate(predicate,
						                                     rightPredicate,
						                                     start);
						break;
					}
					default:
						return predicate;
				}
			}
		}
		
		AST::Node<AST::Predicate> PredicateParser::parseAtomPredicate() {
			const auto start = reader_.position();
			
			switch (reader_.peek().kind()) {
				case Token::TRUEVAL:
					reader_.consume();
					return builder_.makeTruePredicate(start);
				case Token::FALSEVAL:
					reader_.consume();
					return builder_.makeFalsePredicate(start);
				case Token::LROUNDBRACKET: {
					reader_.consume();
					const auto predicate = parsePredicate();
					reader_.expect(Token::RROUNDBRACKET);
					return builder_.makeBracketPredicate(predicate,
					                                     start);
				}
				default:
					break;
			}
			
			const auto symbol = SymbolParser(reader_).parseSymbol(SymbolParser::IN_TYPE);
			switch (reader_.peek().kind()) {
				case Token::RROUNDBRACKET:
				case Token::RTRIBRACKET:
				case Token::AND:
				case Token::OR:
					return builder_.makeSymbolPredicate(symbol, start);
				default:
					break;
			}
			
			auto type = TypeBuilder(reader_).makeSymbolType(symbol, start);
			type = TypeParser(reader_).parseIndirectTypeBasedOnType(type, start);
			
			reader_.expect(Token::COLON);
			
			const auto capabilityType = TypeParser(reader_).parseType();
			
			return builder_.makeTypeSpecPredicate(type, capabilityType, start);
		}
		
	}
	
}
