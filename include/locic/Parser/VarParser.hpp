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
			
			AST::Node<AST::VarList> parseVarList(bool allowInherit);
			
			AST::Node<AST::VarList> parseCStyleVarList();
			
			AST::Node<AST::Var> parseVar(bool allowInherit);
			
			bool scanOptionalToken(Token::Kind kind);
			
			AST::Node<AST::Var> parseTypeVar();
			
			AST::Node<AST::VarList> parseVarOrAnyList();
			
			AST::Node<AST::Var> parseVarOrAny();
			
			AST::Node<AST::Var>
			parseVarWithType(AST::Node<AST::TypeDecl> type,
			                 const Debug::SourcePosition& start);
			
		private:
			TokenReader& reader_;
			VarBuilder builder_;
			
		};
		
	}
	
}

#endif