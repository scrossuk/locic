#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/FunctionParser.hpp>
#include <locic/Parser/SymbolParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeInstanceBuilder.hpp>
#include <locic/Parser/TypeInstanceParser.hpp>
#include <locic/Parser/ValueParser.hpp>
#include <locic/Parser/VarParser.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		class UnexpectedMethodDeclDiag: public Error {
		public:
			UnexpectedMethodDeclDiag() { }
			
			std::string toString() const {
				return "unexpected method declaration; was expecting method definition";
			}
			
		};
		
		class UnexpectedMethodDefDiag: public Error {
		public:
			UnexpectedMethodDefDiag() { }
			
			std::string toString() const {
				return "unexpected method definition; was expecting method declaration";
			}
			
		};
		
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
				case Token::ENUM:
					return parseEnum();
				case Token::EXCEPTION:
					return parseException();
				case Token::INTERFACE:
					return parseInterface();
				case Token::PRIMITIVE:
					return parsePrimitive();
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
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseEnum() {
			const auto start = reader_.position();
			
			reader_.expect(Token::ENUM);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			const auto constructorList = parseEnumConstructorList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeEnum(name, constructorList, start);
		}
		
		AST::Node<AST::StringList> TypeInstanceParser::parseEnumConstructorList() {
			const auto start = reader_.position();
			
			AST::StringList list;
			list.reserve(8);
			
			if (reader_.peek().kind() != Token::RCURLYBRACKET) {
				list.push_back(reader_.expectName());
				
				while (!reader_.isEnd() && reader_.peek().kind() != Token::RCURLYBRACKET) {
					reader_.expect(Token::COMMA);
					list.push_back(reader_.expectName());
				}
			}
			
			return builder_.makeStringList(std::move(list), start);
		}
		
		AST::Node<AST::TypeInstanceList>
		TypeInstanceParser::parseDatatypeVariantList() {
			const auto start = reader_.position();
			
			AST::TypeInstanceList list;
			list.reserve(8);
			
			list.push_back(parseDatatypeVariant());
			
			while (true) {
				if (reader_.peek().kind() != Token::VERTICAL_BAR) {
					break;
				}
				
				reader_.consume();
				list.push_back(parseDatatypeVariant());
			}
			
			return builder_.makeTypeInstanceList(list, start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseDatatypeVariant() {
			const auto start = reader_.position();
			
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LROUNDBRACKET);
			const auto varList = VarParser(reader_).parseVarList();
			reader_.expect(Token::RROUNDBRACKET);
			return builder_.makeDatatype(name, varList, start);
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
		
		AST::Node<AST::ExceptionInitializer>
		TypeInstanceParser::parseExceptionInitializer() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::COLON) {
				return builder_.makeNoneExceptionInitializer(start);
			}
			
			reader_.consume();
			
			const auto symbol = SymbolParser(reader_).parseSymbol(SymbolParser::IN_TYPE);
			
			reader_.expect(Token::LROUNDBRACKET);
			const auto valueList = ValueParser(reader_).parseValueList();
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makeExceptionInitializer(symbol, valueList,
			                                         start);
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
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parsePrimitive() {
			const auto start = reader_.position();
			
			reader_.expect(Token::PRIMITIVE);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			const auto methods = parseMethodDeclList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makePrimitive(name, methods, start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseStruct() {
			const auto start = reader_.position();
			
			reader_.expect(Token::STRUCT);
			const auto name = reader_.expectName();
			
			if (reader_.peek().kind() == Token::SEMICOLON) {
				reader_.consume();
				return builder_.makeOpaqueStruct(name, start);
			}
			
			reader_.expect(Token::LCURLYBRACKET);
			const auto variables = VarParser(reader_).parseCStyleVarList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeStruct(name, variables, start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseUnion() {
			const auto start = reader_.position();
			
			reader_.expect(Token::UNION);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			const auto variables = VarParser(reader_).parseCStyleVarList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeUnion(name, variables, start);
		}
		
		AST::Node<AST::FunctionList> TypeInstanceParser::parseMethodDeclList() {
			const auto start = reader_.position();
			
			AST::FunctionList list;
			list.reserve(8);
			
			while (!reader_.isEnd()) {
				while (reader_.peek().kind() == Token::SEMICOLON) {
					reader_.consume();
				}
				
				if (reader_.peek().kind() == Token::RCURLYBRACKET) {
					break;
				}
				
				const auto function = FunctionParser(reader_).parseMethod();
				
				if (function->isDefinition()) {
					reader_.issueDiagWithLoc(UnexpectedMethodDefDiag(),
					                         function.location());
				}
				
				list.push_back(function);
			}
			
			return builder_.makeFunctionList(std::move(list), start);
		}
		
		AST::Node<AST::FunctionList> TypeInstanceParser::parseMethodDefList() {
			const auto start = reader_.position();
			
			AST::FunctionList list;
			list.reserve(8);
			
			while (!reader_.isEnd()) {
				while (reader_.peek().kind() == Token::SEMICOLON) {
					reader_.consume();
				}
				
				if (reader_.peek().kind() == Token::RCURLYBRACKET) {
					break;
				}
				
				const auto function = FunctionParser(reader_).parseMethod();
				
				if (function->isDeclaration()) {
					reader_.issueDiagWithLoc(UnexpectedMethodDeclDiag(),
					                         function.location());
				}
				
				list.push_back(function);
			}
			
			return builder_.makeFunctionList(std::move(list), start);
		}
		
	}
	
}
