#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/FunctionBuilder.hpp>
#include <locic/Parser/FunctionParser.hpp>
#include <locic/Parser/ScopeParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeParser.hpp>
#include <locic/Parser/VarParser.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		FunctionParser::FunctionParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		FunctionParser::~FunctionParser() { }
		
		AST::Node<AST::Function> FunctionParser::parseGlobalFunction() {
			const auto start = reader_.position();
			
			bool isPrimitive = false;
			bool isImported = false;
			bool isExported = false;
			
			switch (reader_.peek().kind()) {
				case Token::PRIMITIVEFUNCTION:
					reader_.consume();
					isPrimitive = true;
					break;
				case Token::IMPORT:
					reader_.consume();
					isImported = true;
					break;
				case Token::EXPORT:
					reader_.consume();
					isExported = true;
					break;
				default:
					break;
			}
			
			auto function = parseBasicFunction(start);
			
			if (isPrimitive) {
				function->setPrimitive();
			}
			if (isImported) {
				function->setImport();
			}
			if (isExported) {
				function->setExport();
			}
			
			return function;
		}
		
		AST::Node<AST::Function> FunctionParser::parseBasicFunction(const Debug::SourcePosition& start) {
			const auto returnType = TypeParser(reader_).parseType();
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LROUNDBRACKET);
			
			bool isVarArg = false;
			
			const auto varList = VarParser(reader_).parseVarList();
			
			if (reader_.peek().kind() == Token::COMMA) {
				reader_.consume();
				reader_.expect(Token::DOT);
				reader_.expect(Token::DOT);
				reader_.expect(Token::DOT);
				isVarArg = true;
			}
			
			reader_.expect(Token::RROUNDBRACKET);
			
			const auto constSpecifier = parseOptionalConstSpecifier();
			const auto noexceptSpecifier = parseOptionalNoexceptSpecifier();
			const auto requireSpecifier = parseOptionalRequireSpecifier();
			
			if (reader_.peek().kind() == Token::SEMICOLON) {
				reader_.consume();
				return builder_.makeFunctionDecl(returnType, name, varList,
				                                 constSpecifier, noexceptSpecifier,
				                                 requireSpecifier, start);
			}
			
			const auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeFunctionDef(returnType, name, varList,
			                                constSpecifier, noexceptSpecifier,
			                                requireSpecifier, scope, start);
		}
		
		AST::Node<Name> FunctionParser::parseFunctionName() {
			const auto start = reader_.position();
			
			auto name = Name::Relative() + reader_.expectName();
			
			while (true) {
				if (reader_.peek().kind() != Token::DOUBLE_COLON) {
					break;
				}
				
				reader_.consume();
				
				name = name + parseFunctionNameElement();
			}
			
			return builder_.makeName(std::move(name), start);
		}
		
		AST::Node<Name> FunctionParser::parseMethodName() {
			const auto start = reader_.position();
			auto name = Name::Relative() + parseFunctionNameElement();
			return builder_.makeName(std::move(name), start);
		}
		
		String FunctionParser::parseFunctionNameElement() {
			auto validTokens = {
				Token::NAME,
				Token::MOVE,
				Token::NULLVAL
			};
			
			const auto token = reader_.expectOneOf(validTokens);                    
			switch (token.kind()) {
				case Token::NAME:
					return token.name();
				case Token::MOVE:
// 					return stringHost_.getCString("move");
					throw std::logic_error("TODO: method called 'move'");
				case Token::NULLVAL:
// 					return stringHost_.getCString("null");
					throw std::logic_error("TODO: method called 'null'");
				default:
					throw std::logic_error("TODO: invalid method name");
			}
		}
		
	}
	
}
