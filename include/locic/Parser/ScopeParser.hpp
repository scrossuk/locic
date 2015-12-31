#ifndef LOCIC_PARSER_SCOPEPARSER_HPP
#define LOCIC_PARSER_SCOPEPARSER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenReader;
		
		class ScopeParser {
		public:
			ScopeParser(TokenReader& reader);
			~ScopeParser();
			
			AST::Node<AST::Scope> parseScope();
			
			AST::Node<AST::StatementList> parseStatementList();
			
		private:
			TokenReader& reader_;
			
		};
		
	}
	
}

#endif