#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/FunctionParser.hpp>
#include <locic/Parser/NamespaceBuilder.hpp>
#include <locic/Parser/NamespaceParser.hpp>
#include <locic/Parser/TemplateInfo.hpp>
#include <locic/Parser/TemplateParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeInstanceParser.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		NamespaceParser::NamespaceParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		NamespaceParser::~NamespaceParser() { }
		
		AST::Node<AST::NamespaceDecl> NamespaceParser::parseNamespace() {
			const auto start = reader_.position();
			
			reader_.expect(Token::NAMESPACE);
			
			const auto name = reader_.expectName();
			
			reader_.expect(Token::LCURLYBRACKET);
			
			const auto namespaceData = parseNamespaceData();
			
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeNamespace(name, namespaceData, start);
		}
		
		AST::Node<AST::NamespaceData> NamespaceParser::parseNamespaceData() {
			const auto start = reader_.position();
			
			AST::NamespaceData data;
			
			while (true) {
				while (reader_.peek().kind() == Token::SEMICOLON) {
					reader_.consume();
				}
				
				if (reader_.peek().kind() == Token::RCURLYBRACKET) {
					break;
				}
				
				switch (reader_.peek().kind()) {
					case Token::TEMPLATE:
						parseTemplatedObject(data);
						break;
					case Token::USING:
						//data.aliases.push_back(parseAlias());
						throw std::logic_error("TODO: parse alias");
						break;
					case Token::STATIC:
						//data.staticAsserts.push_back(parseStaticAssert());
						throw std::logic_error("TODO: parse static assert");
						break;
					case Token::CLASS:
					case Token::DATATYPE:
					case Token::EXCEPTION:
					case Token::INTERFACE:
					case Token::STRUCT:
					case Token::UNION: {
						auto typeInstance = TypeInstanceParser(reader_).parseTypeInstance();
						data.typeInstances.push_back(typeInstance);
						break;
					}
					case Token::IMPORT:
					case Token::EXPORT:
						if (isNextObjectModuleScope()) {
							data.moduleScopes.push_back(parseModuleScope());
						} else {
							auto function = FunctionParser(reader_).parseGlobalFunction();
							data.functions.push_back(function);
						}
						break;
					case Token::NAMESPACE:
						data.namespaces.push_back(parseNamespace());
						break;
					default: {
						auto function = FunctionParser(reader_).parseGlobalFunction();
						data.functions.push_back(function);
						break;
					}
				}
			}
			
			return builder_.makeNamespaceData(data, start);
		}
		
		void NamespaceParser::parseTemplatedObject(AST::NamespaceData& data) {
			const auto start = reader_.position();
			auto templateInfo = TemplateParser(reader_).parseTemplate();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::CLASS:
				case Token::DATATYPE:
				case Token::EXCEPTION:
				case Token::INTERFACE:
				case Token::STRUCT:
				case Token::UNION: {
					parseTemplatedTypeInstance(data, templateInfo, start);
					break;
				}
				case Token::USING: {
					parseTemplatedAlias(data, templateInfo, start);
					break;
				}
				default: {
					parseTemplatedFunction(data, templateInfo, start);
					break;
				}
			}
		}
		
		bool NamespaceParser::isNextObjectModuleScope() {
			assert(reader_.peek().kind() == Token::IMPORT ||
			       reader_.peek().kind() == Token::EXPORT);
			const auto nextToken = reader_.peek(/*offset=*/1);
			switch (nextToken.kind()) {
				case Token::LCURLYBRACKET:
					// Definitely a module scope.
					return true;
				case Token::NAME:
					// Unknown; could be function or module scope.
					break;
				default:
					// Definitely a function.
					return false;
			}
			
			const auto nextNextToken = reader_.peek(/*offset=*/2);
			switch (nextNextToken.kind()) {
				case Token::DOT:
				case Token::VERSION:
					// Definitely a module scope.
					return true;
				default:
					// Definitely a function.
					return false;
			}
		}
		
		AST::Node<AST::ModuleScope> NamespaceParser::parseModuleScope() {
			const auto start = reader_.position();
			
			const auto token = reader_.expectOneOf({ Token::EXPORT, Token::IMPORT });
			
			if (reader_.peek().kind() == Token::LCURLYBRACKET) {
				reader_.consume();
				
				const auto data = parseNamespaceData();
				
				reader_.expect(Token::RCURLYBRACKET);
				
				if (token.kind() == Token::EXPORT) {
					return builder_.makeUnnamedExport(data, start);
				} else {
					return builder_.makeUnnamedImport(data, start);
				}
			}
			
			throw std::logic_error("TODO: parse named module scope");
		}
		
	}
	
}
