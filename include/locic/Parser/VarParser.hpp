#ifndef LOCIC_PARSER_VARPARSER_HPP
#define LOCIC_PARSER_VARPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/VarBuilder.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class VarParser {
		public:
			VarParser(TokenReader& reader);
			~VarParser();
			
			AST::Node<AST::TypeVarList> parseVarList();
			
			AST::Node<AST::TypeVarList> parseCStyleVarList();
			
			AST::Node<AST::TypeVar> parseVar();
			
			bool scanOptionalToken(Token::Kind kind);
			
			AST::Node<AST::TypeVar> parseTypeVar();
			
			AST::Node<AST::TypeVarList> parseVarOrAnyList();
			
			AST::Node<AST::TypeVar> parseVarOrAny();
			
			AST::Node<AST::TypeVar>
			parseTypeVarWithType(AST::Node<AST::TypeDecl> type,
			                     const Debug::SourcePosition& start);
			
		private:
			TokenReader& reader_;
			VarBuilder builder_;
			
		};
		
	}
	
}

#endif