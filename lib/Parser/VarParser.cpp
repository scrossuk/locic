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
		
		AST::Node<AST::VarList> VarParser::parseVarList() {
			const auto start = reader_.position();
			
			AST::VarList typeVarList;
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
			
			return builder_.makeVarList(std::move(typeVarList), start);
		}
		
		AST::Node<AST::VarList> VarParser::parseCStyleVarList() {
			const auto start = reader_.position();
			
			AST::VarList typeVarList;
			typeVarList.reserve(8);
			
			while (!reader_.isEnd()) {
				while (reader_.peek().kind() == Token::SEMICOLON) {
					reader_.consume();
				}
				
				if (reader_.peek().kind() == Token::RCURLYBRACKET) {
					break;
				}
				
				auto var = VarParser(reader_).parseTypeVar();
				reader_.expect(Token::SEMICOLON);
				typeVarList.push_back(std::move(var));
			}
			
			return builder_.makeVarList(std::move(typeVarList), start);
		}
		
		AST::Node<AST::Var> VarParser::parseVar() {
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
			
			auto symbol = SymbolParser(reader_).parseSymbol(SymbolParser::IN_TYPE);
			
			if (reader_.peek().kind() == Token::LROUNDBRACKET) {
				// This is a pattern variable.
				reader_.consume();
				auto varList = parseVarOrAnyList();
				reader_.expect(Token::RROUNDBRACKET);
				return builder_.makePatternVar(std::move(symbol), std::move(varList), start);
			} else {
				// This is a normal type variable.
				auto symbolType = TypeBuilder(reader_).makeSymbolType(std::move(symbol), start);
				auto type = TypeParser(reader_).parseIndirectTypeBasedOnType(std::move(symbolType), start);
				return parseVarWithType(std::move(type), start);
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
		
		AST::Node<AST::Var> VarParser::parseTypeVar() {
			const auto start = reader_.position();
			auto type = TypeParser(reader_).parseType();
			return parseVarWithType(std::move(type), start);
		}
		
		AST::Node<AST::VarList> VarParser::parseVarOrAnyList() {
			const auto start = reader_.position();
			
			AST::VarList typeVarList;
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
			
			return builder_.makeVarList(std::move(typeVarList), start);
		}
		
		AST::Node<AST::Var> VarParser::parseVarOrAny() {
			if (reader_.peek().kind() == Token::UNDERSCORE) {
				const auto start = reader_.position();
				reader_.consume();
				return builder_.makeAnyVar(start);
			} else {
				return parseVar();
			}
		}
		
		AST::Node<AST::Var>
		VarParser::parseVarWithType(AST::Node<AST::TypeDecl> type,
		                            const Debug::SourcePosition& start) {
			const auto name = reader_.expectName();
			return builder_.makeVar(std::move(type), name, start);
		}
		
	}
	
}
