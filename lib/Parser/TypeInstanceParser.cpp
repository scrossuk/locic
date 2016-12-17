#include <locic/AST.hpp>
#include <locic/Frontend/Diagnostics.hpp>
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
				auto methods = parseMethodDeclList();
				reader_.expect(Token::RCURLYBRACKET);
				return builder_.makeClassDecl(name, std::move(methods), start);
			}
			
			reader_.expect(Token::LROUNDBRACKET);
			auto varList = VarParser(reader_).parseVarList(/*allowInherit=*/true);
			reader_.expect(Token::RROUNDBRACKET);
			
			reader_.expect(Token::LCURLYBRACKET);
			auto methods = parseMethodDefList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeClassDef(name, std::move(varList), std::move(methods), start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseDatatype() {
			const auto start = reader_.position();
			
			reader_.expect(Token::DATATYPE);
			const auto name = reader_.expectName();
			
			if (reader_.peek().kind() == Token::LROUNDBRACKET) {
				reader_.consume();
				auto varList = VarParser(reader_).parseVarList(/*allowInherit=*/false);
				reader_.expect(Token::RROUNDBRACKET);
				return builder_.makeDatatype(name, std::move(varList), start);
			}
			
			reader_.expect(Token::SETEQUAL);
			
			auto variants = parseDatatypeVariantList();
			
			return builder_.makeUnionDatatype(name, std::move(variants), start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseEnum() {
			const auto start = reader_.position();
			
			reader_.expect(Token::ENUM);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			auto constructorList = parseEnumConstructorList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeEnum(name, std::move(constructorList), start);
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
			
			return builder_.makeTypeInstanceList(std::move(list), start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseDatatypeVariant() {
			const auto start = reader_.position();
			
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LROUNDBRACKET);
			auto varList = VarParser(reader_).parseVarList(/*allowInherit=*/false);
			reader_.expect(Token::RROUNDBRACKET);
			return builder_.makeDatatype(name, std::move(varList), start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseException() {
			const auto start = reader_.position();
			
			reader_.expect(Token::EXCEPTION);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LROUNDBRACKET);
			auto varList = VarParser(reader_).parseVarList(/*allowInherit=*/false);
			reader_.expect(Token::RROUNDBRACKET);
			
			auto initializer = parseExceptionInitializer();
			
			return builder_.makeException(name, std::move(varList), std::move(initializer), start);
		}
		
		AST::Node<AST::ExceptionInitializer>
		TypeInstanceParser::parseExceptionInitializer() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::COLON) {
				return builder_.makeNoneExceptionInitializer(start);
			}
			
			reader_.consume();
			
			auto symbol = SymbolParser(reader_).parseSymbol(SymbolParser::IN_TYPE);
			
			reader_.expect(Token::LROUNDBRACKET);
			auto valueList = ValueParser(reader_).parseValueList();
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makeExceptionInitializer(std::move(symbol), std::move(valueList),
			                                         start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseInterface() {
			const auto start = reader_.position();
			
			reader_.expect(Token::INTERFACE);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			auto methods = parseMethodDeclList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeInterface(name, std::move(methods), start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parsePrimitive() {
			const auto start = reader_.position();
			
			reader_.expect(Token::PRIMITIVE);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			auto methods = parseMethodDeclList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makePrimitive(name, std::move(methods), start);
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
			auto variables = VarParser(reader_).parseCStyleVarList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeStruct(name, std::move(variables), start);
		}
		
		AST::Node<AST::TypeInstance> TypeInstanceParser::parseUnion() {
			const auto start = reader_.position();
			
			reader_.expect(Token::UNION);
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			auto variables = VarParser(reader_).parseCStyleVarList();
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeUnion(name, std::move(variables), start);
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
				
				auto function = FunctionParser(reader_).parseMethod();
				
				if (function->hasScopeDecl() ||
				    function->isAutoGenerated()) {
					reader_.issueDiagWithLoc(UnexpectedMethodDefDiag(),
					                         function.location());
				}
				
				list.push_back(std::move(function));
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
				
				auto function = FunctionParser(reader_).parseMethod();
				
				if (!function->hasScopeDecl() &&
				    !function->isAutoGenerated()) {
					reader_.issueDiagWithLoc(UnexpectedMethodDeclDiag(),
					                         function.location());
				}
				
				list.push_back(std::move(function));
			}
			
			return builder_.makeFunctionList(std::move(list), start);
		}
		
	}
	
}
