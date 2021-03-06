#include <locic/AST.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Parser/AttributeBuilder.hpp>
#include <locic/Parser/AttributeParser.hpp>
#include <locic/Parser/PredicateParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		AttributeParser::AttributeParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		AttributeParser::~AttributeParser() { }
			
		AST::Node<AST::ConstSpecifier>
		AttributeParser::parseOptionalConstSpecifier() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::CONST) {
				return builder_.makeNeverConstSpecifier(start);
			}
			
			reader_.consume();
			
			if (reader_.peek().kind() != Token::LROUNDBRACKET) {
				return builder_.makeAlwaysConstSpecifier(start);
			}
			
			reader_.consume();
			
			auto predicate = PredicateParser(reader_).parsePredicate();
			
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makePredicateConstSpecifier(std::move(predicate), start);
		}
		
		AST::Node<AST::RequireSpecifier>
		AttributeParser::parseOptionalMoveSpecifier() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::MOVE) {
				return builder_.makeNeverRequireSpecifier(start);
			}
			
			reader_.consume();
			
			if (reader_.peek().kind() != Token::LROUNDBRACKET) {
				return builder_.makeAlwaysRequireSpecifier(start);
			}
			
			reader_.consume();
			
			auto predicate = PredicateParser(reader_).parsePredicate();
			
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makePredicateRequireSpecifier(std::move(predicate), start);
		}
		
		AST::Node<AST::RequireSpecifier>
		AttributeParser::parseOptionalNoexceptSpecifier() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::NOEXCEPT) {
				return builder_.makeNeverRequireSpecifier(start);
			}
			
			reader_.consume();
			
			if (reader_.peek().kind() != Token::LROUNDBRACKET) {
				return builder_.makeAlwaysRequireSpecifier(start);
			}
			
			reader_.consume();
			
			auto predicate = PredicateParser(reader_).parsePredicate();
			
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makePredicateRequireSpecifier(std::move(predicate), start);
		}
		
		AST::Node<AST::RequireSpecifier>
		AttributeParser::parseOptionalRequireSpecifier() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::REQUIRE) {
				return builder_.makeNeverRequireSpecifier(start);
			}
			
			reader_.consume();
			
			reader_.expect(Token::LROUNDBRACKET);
			auto predicate = PredicateParser(reader_).parsePredicate();
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makePredicateRequireSpecifier(std::move(predicate), start);
		}
		
	}
	
}
