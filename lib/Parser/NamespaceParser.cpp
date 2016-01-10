#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/FunctionParser.hpp>
#include <locic/Parser/NamespaceBuilder.hpp>
#include <locic/Parser/NamespaceParser.hpp>
#include <locic/Parser/PredicateParser.hpp>
#include <locic/Parser/TemplateInfo.hpp>
#include <locic/Parser/TemplateParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeInstanceParser.hpp>
#include <locic/Parser/ValueParser.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		NamespaceParser::NamespaceParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		NamespaceParser::~NamespaceParser() { }
		
		AST::Node<AST::NamespaceDecl> NamespaceParser::parseGlobalNamespace() {
			const auto start = reader_.position();
			const auto namespaceData = parseNamespaceData();
			return builder_.makeNamespace(reader_.makeCString(""), namespaceData, start);
		}
		
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
				
				if (reader_.peek().kind() == Token::RCURLYBRACKET ||
				    reader_.peek().kind() == Token::END) {
					break;
				}
				
				switch (reader_.peek().kind()) {
					case Token::TEMPLATE:
						parseTemplatedObject(data);
						break;
					case Token::USING:
						data.aliases.push_back(parseAlias());
						break;
					case Token::STATIC:
						if (reader_.peek(/*offset=*/1).kind() == Token::ASSERT) {
							data.staticAsserts.push_back(parseStaticAssert());
						} else {
							auto function = FunctionParser(reader_).parseGlobalFunction();
							data.functions.push_back(function);
						}
						break;
					case Token::ENUM:
					case Token::CLASS:
					case Token::DATATYPE:
					case Token::EXCEPTION:
					case Token::INTERFACE:
					case Token::PRIMITIVE:
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
				case Token::PRIMITIVE:
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
		
		void NamespaceParser::parseTemplatedTypeInstance(AST::NamespaceData& data,
		                                                 TemplateInfo templateInfo,
		                                                 const Debug::SourcePosition& start) {
			auto typeInstance = TypeInstanceParser(reader_).parseTypeInstance();
			
			typeInstance->setTemplateVariables(templateInfo.templateVariables());
			
			if (templateInfo.hasRequireSpecifier()) {
				typeInstance->setRequireSpecifier(templateInfo.requireSpecifier());
			}
			if (templateInfo.hasMoveSpecifier()) {
				typeInstance->setMoveSpecifier(templateInfo.moveSpecifier());
			}
			if (templateInfo.hasNoTagSet()) {
				typeInstance->setNoTagSet(templateInfo.noTagSet());
			}
			
			typeInstance.setLocation(reader_.locationWithRangeFrom(start));
			
			data.typeInstances.push_back(typeInstance);
		}
		
		void NamespaceParser::parseTemplatedAlias(AST::NamespaceData& data,
		                                          TemplateInfo templateInfo,
		                                          const Debug::SourcePosition& start) {
			auto alias = parseAlias();
			
			alias->setTemplateVariables(templateInfo.templateVariables());
			
			if (templateInfo.hasRequireSpecifier()) {
				alias->setRequireSpecifier(templateInfo.requireSpecifier());
			}
			
			alias.setLocation(reader_.locationWithRangeFrom(start));
			
			data.aliases.push_back(alias);
		}
		
		AST::Node<AST::AliasDecl> NamespaceParser::parseAlias() {
			const auto start = reader_.position();
			
			reader_.expect(Token::USING);
			const auto name = reader_.expectName();
			reader_.expect(Token::SETEQUAL);
			const auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::SEMICOLON);
			
			return builder_.makeAlias(name, value, start);
		}
		
		AST::Node<AST::StaticAssert> NamespaceParser::parseStaticAssert() {
			const auto start = reader_.position();
			
			reader_.expect(Token::STATIC);
			reader_.expect(Token::ASSERT);
			const auto predicate = PredicateParser(reader_).parsePredicate();
			reader_.expect(Token::SEMICOLON);
			
			return builder_.makeStaticAssert(predicate, start);
		}
		
		void NamespaceParser::parseTemplatedFunction(AST::NamespaceData& data,
		                                             TemplateInfo templateInfo,
		                                             const Debug::SourcePosition& start) {
			auto function = FunctionParser(reader_).parseGlobalFunction();
			
			function->setTemplateVariables(templateInfo.templateVariables());
			
			if (templateInfo.hasRequireSpecifier()) {
				// TODO: reject duplicate require() specifier.
				function->setRequireSpecifier(templateInfo.requireSpecifier());
			}
			
			// TODO: reject move() or notag().
			
			function.setLocation(reader_.locationWithRangeFrom(start));
			
			data.functions.push_back(function);
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
			
			const auto moduleName = parseModuleName();
			const auto version = parseModuleVersion();
			
			reader_.expect(Token::LCURLYBRACKET);
			const auto data = parseNamespaceData();
			reader_.expect(Token::RCURLYBRACKET);
			
			if (token.kind() == Token::EXPORT) {
				return builder_.makeNamedExport(moduleName, version,
				                                data, start);
			} else {
				return builder_.makeNamedImport(moduleName, version,
				                                data, start);
			}
		}
		
		AST::Node<AST::StringList> NamespaceParser::parseModuleName() {
			const auto start = reader_.position();
			
			AST::StringList list;
			list.reserve(4);
			list.push_back(reader_.expectName());
			
			while (reader_.peek().kind() == Token::DOT) {
				reader_.consume();
				list.push_back(reader_.expectName());
			}
			
			return builder_.makeStringList(std::move(list), start);
		}
		
		AST::Node<Version> NamespaceParser::parseModuleVersion() {
			const auto start = reader_.position();
			const auto version = reader_.expectVersion();
			return builder_.makeVersion(version, start);
		}
		
	}
	
}
