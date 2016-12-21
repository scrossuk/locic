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
			
			AST::Node<AST::StatementDecl> parseStatement();
			
			AST::Node<AST::StatementDecl> parseScopeStatement();
			
			AST::Node<AST::StatementDecl> parseIfStatement();
			
			AST::Node<AST::IfClause> parseIfClause();
			
			AST::Node<AST::StatementDecl> parseSwitchStatement();
			
			AST::Node<AST::SwitchCaseList> parseSwitchCaseList();
			
			AST::Node<AST::SwitchCase> parseSwitchCase();
			
			AST::Node<AST::DefaultCase> parseSwitchDefaultCase();
			
			AST::Node<AST::StatementDecl> parseWhileStatement();
			
			AST::Node<AST::StatementDecl> parseForStatement();
			
			AST::Node<AST::StatementDecl> parseTryStatement();
			
			AST::Node<AST::CatchClauseList> parseCatchClauseList();
			
			AST::Node<AST::CatchClause> parseCatchClause();
			
			AST::Node<AST::StatementDecl> parseScopeExitStatement();
			
			AST::Node<AST::StatementDecl> parseAssertStatement();
			
			bool isVarDeclStartToken(Token::Kind kind);
			
			AST::Node<AST::StatementDecl> parseVarDeclStatement();
			
			AST::Node<AST::StatementDecl> parseValueOrVarDeclStatement();
			
		private:
			TokenReader& reader_;
			StatementBuilder builder_;
			
		};
		
	}
	
}

#endif