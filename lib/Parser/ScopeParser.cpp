#include <locic/AST.hpp>
#include <locic/Parser/ScopeParser.hpp>
#include <locic/Parser/StatementParser.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		ScopeParser::ScopeParser(TokenReader& reader)
		: reader_(reader) { }
		
		ScopeParser::~ScopeParser() { }
		
		AST::Node<AST::Scope> ScopeParser::parseScope() {
			const auto start = reader_.position();
			
			reader_.expect(Token::LCURLYBRACKET);
			auto statementList = parseStatementList();
			reader_.expect(Token::RCURLYBRACKET);
			
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::Scope(std::move(statementList)));
		}
		
		AST::Node<AST::StatementDeclList> ScopeParser::parseStatementList() {
			const auto start = reader_.position();
			
			AST::StatementDeclList statementList;
			statementList.reserve(16);
			
			while (!reader_.isEnd()) {
				while (reader_.peek().kind() == Token::SEMICOLON) {
					reader_.consume();
				}
				
				if (reader_.peek().kind() == Token::RCURLYBRACKET) {
					break;
				}
				
				auto statement = StatementParser(reader_).parseStatement();
				statementList.push_back(std::move(statement));
			}
			
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::StatementDeclList(std::move(statementList)));
		}
		
	}
	
}
