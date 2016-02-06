#include <locic/AST.hpp>
#include <locic/Frontend/Diagnostics.hpp>
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
						auto rightPredicate = parseAtomPredicate();
						predicate = builder_.makeAndPredicate(std::move(predicate),
						                                      std::move(rightPredicate),
						                                      start);
						break;
					}
					case Token::OR: {
						reader_.consume();
						auto rightPredicate = parseAtomPredicate();
						predicate = builder_.makeOrPredicate(std::move(predicate),
						                                     std::move(rightPredicate),
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
					auto predicate = parsePredicate();
					reader_.expect(Token::RROUNDBRACKET);
					return builder_.makeBracketPredicate(std::move(predicate),
					                                     start);
				}
				default:
					break;
			}
			
			auto symbol = SymbolParser(reader_).parseSymbol(SymbolParser::IN_TYPE);
			switch (reader_.peek().kind()) {
				case Token::RROUNDBRACKET:
				case Token::RTRIBRACKET:
				case Token::AND:
				case Token::OR:
				case Token::SEMICOLON:
					return builder_.makeSymbolPredicate(std::move(symbol), start);
				default:
					break;
			}
			
			auto type = TypeBuilder(reader_).makeSymbolType(std::move(symbol), start);
			type = TypeParser(reader_).parseIndirectTypeBasedOnType(std::move(type), start);
			
			reader_.expect(Token::COLON);
			
			auto capabilityType = TypeParser(reader_).parseType();
			
			return builder_.makeTypeSpecPredicate(std::move(type), std::move(capabilityType), start);
		}
		
	}
	
}
