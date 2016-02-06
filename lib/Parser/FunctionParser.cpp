#include <locic/AST.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Parser/AttributeParser.hpp>
#include <locic/Parser/FunctionBuilder.hpp>
#include <locic/Parser/FunctionParser.hpp>
#include <locic/Parser/ScopeParser.hpp>
#include <locic/Parser/TemplateInfo.hpp>
#include <locic/Parser/TemplateParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeParser.hpp>
#include <locic/Parser/VarParser.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		class StaticMethodCannotBeConstDiag: public Warning {
		public:
			StaticMethodCannotBeConstDiag() { }
			
			std::string toString() const {
				return "static method cannot have const predicate";
			}
			
		};
		
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
			const bool isStatic = reader_.consumeIfPresent(Token::STATIC);
			auto returnType = TypeParser(reader_).parseType();
			auto name = parseFunctionName();
			
			reader_.expect(Token::LROUNDBRACKET);
			
			bool isVarArg = false;
			
			auto varList = VarParser(reader_).parseVarList();
			
			if (reader_.peek().kind() == Token::COMMA) {
				reader_.consume();
				reader_.expect(Token::DOT);
				reader_.expect(Token::DOT);
				reader_.expect(Token::DOT);
				isVarArg = true;
			}
			
			reader_.expect(Token::RROUNDBRACKET);
			
			auto constSpecifier = AttributeParser(reader_).parseOptionalConstSpecifier();
			auto noexceptSpecifier = AttributeParser(reader_).parseOptionalNoexceptSpecifier();
			auto requireSpecifier = AttributeParser(reader_).parseOptionalRequireSpecifier();
			
			if (isStatic && !constSpecifier->isNone()) {
				reader_.issueDiagWithLoc(StaticMethodCannotBeConstDiag(),
				                         constSpecifier.location());
			}
			
			if (reader_.peek().kind() == Token::SEMICOLON) {
				reader_.consume();
				return builder_.makeFunctionDecl(isVarArg, isStatic,
				                                 std::move(returnType), std::move(name), std::move(varList),
				                                 std::move(constSpecifier), std::move(noexceptSpecifier),
				                                 std::move(requireSpecifier), start);
			}
			
			auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeFunctionDef(isVarArg, isStatic, std::move(returnType),
			                                std::move(name), std::move(varList), std::move(constSpecifier),
			                                std::move(noexceptSpecifier), std::move(requireSpecifier),
			                                std::move(scope), start);
		}
		
		AST::Node<AST::Function> FunctionParser::parseMethod() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::TEMPLATE) {
				return parseNonTemplatedMethod(start);
			}
			
			auto templateInfo = TemplateParser(reader_).parseTemplate();
			auto function = parseNonTemplatedMethod(start);
			
			function->setTemplateVariables(templateInfo.extractTemplateVariables());
			
			if (templateInfo.hasRequireSpecifier()) {
				// TODO: reject duplicate require() specifier.
				function->setRequireSpecifier(templateInfo.extractRequireSpecifier());
			}
			
			// TODO: reject move() or notag().
			return function;
		}
		
		AST::Node<AST::Function>
		FunctionParser::parseNonTemplatedMethod(const Debug::SourcePosition& start) {
			if (reader_.peek().kind() == Token::TILDA) {
				reader_.consume();
				const auto nameString = reader_.makeCString("__destroy");
				auto name = builder_.makeName(Name::Relative() + nameString, start);
				auto scope = ScopeParser(reader_).parseScope();
				return builder_.makeDestructor(std::move(name), std::move(scope), start);
			}
			
			const bool isStatic = reader_.consumeIfPresent(Token::STATIC);
			auto returnType = parseMethodReturnType();
			auto name = parseMethodName();
			
			if (reader_.peek().kind() == Token::SETEQUAL) {
				reader_.consume();
				reader_.expect(Token::DEFAULT);
				auto requireSpecifier = AttributeParser(reader_).parseOptionalRequireSpecifier();
				reader_.expect(Token::SEMICOLON);
				return builder_.makeDefaultMethod(isStatic, std::move(name), std::move(requireSpecifier), start);
			}
			
			reader_.expect(Token::LROUNDBRACKET);
			auto varList = VarParser(reader_).parseVarList();
			reader_.expect(Token::RROUNDBRACKET);
			
			auto constSpecifier = AttributeParser(reader_).parseOptionalConstSpecifier();
			auto noexceptSpecifier = AttributeParser(reader_).parseOptionalNoexceptSpecifier();
			auto requireSpecifier = AttributeParser(reader_).parseOptionalRequireSpecifier();
			
			if (isStatic && !constSpecifier->isNone()) {
				reader_.issueDiagWithLoc(StaticMethodCannotBeConstDiag(),
				                         constSpecifier.location());
			}
			
			if (reader_.peek().kind() == Token::SEMICOLON) {
				reader_.consume();
				return builder_.makeFunctionDecl(/*isVarArg=*/false, isStatic,
				                                 std::move(returnType), std::move(name), std::move(varList),
				                                 std::move(constSpecifier), std::move(noexceptSpecifier),
				                                 std::move(requireSpecifier), start);
			}
			
			auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeFunctionDef(/*isVarArg=*/false, isStatic,
			                                std::move(returnType), std::move(name), std::move(varList),
			                                std::move(constSpecifier), std::move(noexceptSpecifier),
			                                std::move(requireSpecifier), std::move(scope), start);
		}
		
		AST::Node<AST::Type>
		FunctionParser::parseMethodReturnType() {
			if (isValidMethodNameToken(reader_.peek().kind())) {
				const auto nextToken = reader_.peek(/*offset=*/1);
				switch (nextToken.kind()) {
					case Token::SETEQUAL:
					case Token::LROUNDBRACKET:
						return TypeBuilder(reader_).makeAutoType(reader_.position());
					default:
						break;
				}
			}
			
			return TypeParser(reader_).parseType();
		}
		
		bool FunctionParser::isValidMethodNameToken(const Token::Kind kind) const {
			switch (kind) {
				case Token::NAME:
				case Token::MOVE:
				case Token::NULLVAL:
					return true;
				default:
					return false;
			}
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
					return reader_.makeCString("move");
				case Token::NULLVAL:
					return reader_.makeCString("null");
				default:
					return reader_.makeCString("<invalid>");
			}
		}
		
	}
	
}
