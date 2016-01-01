#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/PredicateBuilder.hpp>
#include <locic/Parser/PredicateParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>

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
			
			const auto leftPredicate = parseAtomPredicate();
			
			switch (reader_.peek().kind()) {
				case Token::AND: {
					reader_.consume();
					const auto rightPredicate = parseAtomPredicate();
					return builder_.makeAndPredicate(leftPredicate,
					                                 rightPredicate,
					                                 start);
				}
				case Token::OR: {
					reader_.consume();
					const auto rightPredicate = parseAtomPredicate();
					return builder_.makeOrPredicate(leftPredicate,
					                                rightPredicate,
					                                start);
				}
				default:
					return leftPredicate;
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
			
			throw std::logic_error("TODO: (type:type) or symbol");
		}
		
	}
	
}
