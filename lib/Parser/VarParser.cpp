#include <locic/AST.hpp>
#include <locic/Parser/SymbolParser.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeBuilder.hpp>
#include <locic/Parser/TypeParser.hpp>
#include <locic/Parser/VarParser.hpp>

namespace locic {
	
	namespace Parser {
		
		VarParser::VarParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		VarParser::~VarParser() { }
		
		AST::Node<AST::TypeVar> VarParser::parseVar() {
			const auto start = reader_.position();
			
			const bool isUnused = scanOptionalToken(Token::UNUSED);
			const bool isFinal = scanOptionalToken(Token::FINAL);
			const bool isOverrideConst = scanOptionalToken(Token::OVERRIDE_CONST);
			
			if (isUnused || isFinal || isOverrideConst) {
				// Simple case: this isn't a pattern.
				auto typeVar = parseTypeVar();
				if (isUnused) {
					typeVar->setUnused();
				}
				if (isFinal) {
					typeVar->setFinal();
				}
				if (isOverrideConst) {
					typeVar->setOverrideConst();
				}
				// TODO: fix location.
				return typeVar;
			}
			
			if (reader_.peek().kind() != Token::NAME) {
				return parseTypeVar();
			}
			
			const auto symbol = SymbolParser(reader_).parseSymbol();
			
			if (reader_.peek().kind() == Token::LROUNDBRACKET) {
				// This is a pattern variable.
				throw std::logic_error("TODO");
			} else {
				// This is a normal type variable.
				const auto symbolType = TypeBuilder(reader_).makeSymbolType(symbol, start);
				const auto type = TypeParser(reader_).parseIndirectTypeBasedOnType(symbolType, start);
				return parseTypeVarWithType(type, start);
			}
		}
		
		bool VarParser::scanOptionalToken(const Token::Kind kind) {
			if (reader_.peek().kind() == kind) {
				reader_.consume();
				return true;
			} else {
				return false;
			}
		}
		
		AST::Node<AST::TypeVar> VarParser::parseTypeVar() {
			const auto start = reader_.position();
			const auto type = TypeParser(reader_).parseType();
			return parseTypeVarWithType(type, start);
		}
		
		AST::Node<AST::TypeVar> VarParser::parseVarOrAny() {
			if (reader_.peek().kind() == Token::UNDERSCORE) {
				const auto start = reader_.position();
				reader_.consume();
				return builder_.makeAnyVar(start);
			} else {
				return parseVar();
			}
		}
		
		AST::Node<AST::TypeVar> VarParser::parseTypeVarWithType(AST::Node<AST::Type> type,
		                                                        const Debug::SourcePosition& start) {
			const auto nameToken = reader_.peek();
			
			String name;
			if (nameToken.kind() != Token::NAME) {
				throw std::logic_error("TODO: didn't find NAME.");
			} else {
				name = nameToken.name();
				reader_.consume();
			}
			
			return builder_.makeTypeVar(type, name, start);
		}
		
	}
	
}
