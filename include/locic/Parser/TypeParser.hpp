#ifndef LOCIC_PARSER_TYPEPARSER_HPP
#define LOCIC_PARSER_TYPEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TypeBuilder.hpp>

namespace locic {
	
	class PrimitiveID;
	class StringHost;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class TypeParser {
		public:
			TypeParser(TokenReader& reader);
			~TypeParser();
			
			AST::Node<AST::TypeDecl> parseType();
			
			AST::Node<AST::TypeDecl> parseIndirectTypeBasedOnType(AST::Node<AST::TypeDecl> type,
			                                                  const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl> parseQualifiedType();
			
			AST::Node<AST::TypeDecl> parseConstType();
			
			AST::Node<AST::TypeDecl> parseFunctionPointerType();
			
			AST::Node<AST::TypeDeclList> parseTypeList();
			
			AST::Node<AST::TypeDecl> parseBasicType();
			
			AST::Node<AST::TypeDecl> parseLongIntOrFloatType(const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl> parseIntegerTypeWithSignedness(const Debug::SourcePosition& start,
			                                                    bool isSigned);
			
			AST::Node<AST::TypeDecl> parseLongIntegerType(const Debug::SourcePosition& start,
			                                          bool isSigned);
			
			bool isTypeStartToken(Token::Kind kind);
			
		private:
			TokenReader& reader_;
			TypeBuilder builder_;
			
		};
		
	}
	
}

#endif
