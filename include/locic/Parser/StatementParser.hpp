#ifndef LOCIC_PARSER_STATEMENTPARSER_HPP
#define LOCIC_PARSER_STATEMENTPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/StatementBuilder.hpp>
#include <locic/Parser/Token.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenReader;
		
		class StatementParser {
		public:
			StatementParser(TokenReader& reader);
			~StatementParser();
			
			AST::Node<AST::Statement> parseStatement();
			
			AST::Node<AST::Statement> parseScopeStatement();
			
			AST::Node<AST::Statement> parseIfStatement();
			
			AST::Node<AST::IfClause> parseIfClause();
			
			AST::Node<AST::Statement> parseSwitchStatement();
			
			AST::Node<AST::SwitchCaseList> parseSwitchCaseList();
			
			AST::Node<AST::SwitchCase> parseSwitchCase();
			
			AST::Node<AST::DefaultCase> parseSwitchDefaultCase();
			
			AST::Node<AST::Statement> parseWhileStatement();
			
			AST::Node<AST::Statement> parseForStatement();
			
			AST::Node<AST::Statement> parseAssertStatement();
			
			bool isVarDeclStartToken(Token::Kind kind);
			
			AST::Node<AST::Statement> parseVarDeclStatement();
			
			AST::Node<AST::Statement> parseValueOrVarDeclStatement();
			
			AST::Node<AST::Type> interpretValueAsType(const AST::Node<AST::Value>& value);
			
		private:
			TokenReader& reader_;
			StatementBuilder builder_;
			
		};
		
	}
	
}

#endif