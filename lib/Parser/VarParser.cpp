#include <locic/AST.hpp>
#include <locic/Frontend/Diagnostics.hpp>
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
		
		AST::Node<AST::TypeVarList> VarParser::parseVarList() {
			const auto start = reader_.position();
			
			AST::TypeVarList typeVarList;
			typeVarList.reserve(8);
			
			if (reader_.peek().kind() != Token::RROUNDBRACKET) {
				typeVarList.push_back(parseVar());
				
				while (true) {
					if (reader_.peek().kind() != Token::COMMA) {
						break;
					}
					
					if (reader_.peek(/*offset=*/1).kind() == Token::DOT) {
						// This will be a var args ellipsis,
						// so don't consume it.
						break;
					}
					
					reader_.consume();
					
					typeVarList.push_back(parseVar());
				}
			}
			
			return builder_.makeTypeVarList(std::move(typeVarList), start);
		}
		
		AST::Node<AST::TypeVarList> VarParser::parseCStyleVarList() {
			const auto start = reader_.position();
			
			AST::TypeVarList typeVarList;
			typeVarList.reserve(8);
			
			while (!reader_.isEnd()) {
				while (reader_.peek().kind() == Token::SEMICOLON) {
					reader_.consume();
				}
				
				if (reader_.peek().kind() == Token::RCURLYBRACKET) {
					break;
				}
				
				const auto var = VarParser(reader_).parseTypeVar();
				reader_.expect(Token::SEMICOLON);
				typeVarList.push_back(var);
			}
			
			return builder_.makeTypeVarList(std::move(typeVarList), start);
		}
		
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
			
			const auto symbol = SymbolParser(reader_).parseSymbol(SymbolParser::IN_TYPE);
			
			if (reader_.peek().kind() == Token::LROUNDBRACKET) {
				// This is a pattern variable.
				reader_.consume();
				const auto varList = parseVarOrAnyList();
				reader_.expect(Token::RROUNDBRACKET);
				return builder_.makePatternVar(symbol, varList, start);
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
		
		AST::Node<AST::TypeVarList> VarParser::parseVarOrAnyList() {
			const auto start = reader_.position();
			
			AST::TypeVarList typeVarList;
			typeVarList.reserve(8);
			
			if (reader_.peek().kind() != Token::RROUNDBRACKET) {
				typeVarList.push_back(parseVarOrAny());
				
				while (true) {
					if (reader_.peek().kind() != Token::COMMA) {
						break;
					}
					
					reader_.consume();
					
					typeVarList.push_back(parseVarOrAny());
				}
			}
			
			return builder_.makeTypeVarList(std::move(typeVarList), start);
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
			const auto name = reader_.expectName();
			return builder_.makeTypeVar(type, name, start);
		}
		
	}
	
}
