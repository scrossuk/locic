#include <locic/AST.hpp>
#include <locic/Parser/AttributeParser.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/TemplateInfo.hpp>
#include <locic/Parser/TemplateBuilder.hpp>
#include <locic/Parser/TemplateParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeParser.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		TemplateParser::TemplateParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		TemplateParser::~TemplateParser() { }
		
		TemplateInfo TemplateParser::parseTemplate() {
			TemplateInfo info;
			
			reader_.expect(Token::TEMPLATE);
			reader_.expect(Token::LTRIBRACKET);
			
			info.setTemplateVariables(parseTemplateVarList());
			
			reader_.expect(Token::RTRIBRACKET);
			
			AttributeParser attributeParser(reader_);
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::REQUIRE:
						info.setRequireSpecifier(attributeParser.parseOptionalRequireSpecifier());
						break;
					case Token::MOVE:
						info.setMoveSpecifier(attributeParser.parseOptionalMoveSpecifier());
						break;
					case Token::NOTAG:
						info.setNoTagSet(attributeParser.parseOptionalNoTagSet());
						break;
					default:
						return info;
				}
			}
		}
		
		AST::Node<AST::TemplateTypeVarList>
		TemplateParser::parseTemplateVarList() {
			const auto start = reader_.position();
			
			AST::TemplateTypeVarList varList;
			varList.reserve(8);
			
			if (reader_.peek().kind() != Token::RTRIBRACKET) {
				varList.push_back(parseTemplateVar());
				while (true) {
					if (reader_.peek().kind() != Token::COMMA) {
						break;
					}
					
					reader_.consume();
					
					varList.push_back(parseTemplateVar());
				}
			}
			
			return builder_.makeTemplateVarList(std::move(varList), start);
		}
		
		AST::Node<AST::TemplateTypeVar>
		TemplateParser::parseTemplateVar() {
			const auto start = reader_.position();
			
			const auto type = TypeParser(reader_).parseType();
			const auto name = reader_.expectName();
			
			if (reader_.peek().kind() != Token::COLON) {
				return builder_.makeTemplateVar(type, name, start);
			}
			
			reader_.consume();
			
			const auto capabilityType = TypeParser(reader_).parseType();
			
			return builder_.makeCapabilityTemplateVar(type, name,
			                                          capabilityType,
			                                          start);
		}
		
	}
	
}
