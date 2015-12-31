#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeInstanceBuilder.hpp>
#include <locic/Parser/TypeInstanceParser.hpp>
#include <locic/Parser/VarParser.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		TypeInstanceParser::TypeInstanceParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		TypeInstanceParser::~TypeInstanceParser() { }
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseTypeInstance() {
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::CLASS:
					return parseClass();
				case Token::DATATYPE:
					return parseDatatype();
				case Token::EXCEPTION:
					return parseException();
				case Token::INTERFACE:
					return parseInterface();
				case Token::STRUCT:
					return parseStruct();
				case Token::UNION:
					return parseUnion();
				default:
					throw std::logic_error("TODO: invalid type instance");
			}
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseClass() {
			const auto start = reader_.position();
			
			reader_.expect(Token::CLASS);
			const auto name = reader_.expectName();
			
			if (reader_.peek().kind() == Token::LCURLYBRACKET) {
				reader_.consume();
				const auto methods = parseMethodDeclList();
				reader_.expect(Token::RCURLYBRACKET);
				return builder_.makeClassDecl(name, methods, start);
			}
			
			reader_.expect(Token::LROUNDBRACKET);
			const auto varList = VarParser(reader_).parseVarList();
			reader_.expect(Token::RROUNDBRACKET);
			
			reader_.expect(Token::LCURLYBRACKET);
			const auto methods = parseMethodDefList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeClassDef(name, varList, methods, start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseDatatype() {
			const auto start = reader_.position();
			
			reader_.expect(Token::DATATYPE);
			const auto name = reader_.expectName();
			
			if (reader_.peek().kind() == Token::LROUNDBRACKET) {
				reader_.consume();
				const auto varList = VarParser(reader_).parseVarList();
				reader_.expect(Token::RROUNDBRACKET);
				return builder_.makeDatatype(name, varList, start);
			}
			
			reader_.expect(Token::SETEQUAL);
			
			const auto variants = parseDatatypeVariantList();
			
			return builder_.makeUnionDatatype(name, variants, start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseException() {
			const auto start = reader_.position();
			
			reader_.expect(Token::EXCEPTION);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LROUNDBRACKET);
			const auto varList = VarParser(reader_).parseVarList();
			reader_.expect(Token::RROUNDBRACKET);
			
			const auto initializer = parseExceptionInitializer();
			
			return builder_.makeException(name, varList, initializer, start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseInterface() {
			const auto start = reader_.position();
			
			reader_.expect(Token::INTERFACE);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			const auto methods = parseMethodDeclList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeInterface(name, methods, start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseStruct() {
			const auto start = reader_.position();
			
			reader_.expect(Token::STRUCT);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			const auto variables = VarParser(reader_).parseCStyleVarList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeStruct(name, variables, start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseUnion() {
			throw std::logic_error("TODO: parse union");
		}
		
	}
	
}
