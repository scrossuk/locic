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
			
			AST::Node<AST::Type> parseType();
			
			AST::Node<AST::Type> parseIndirectTypeBasedOnType(AST::Node<AST::Type> type,
			                                                  const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> parseQualifiedType();
			
			AST::Node<AST::Type> parseConstType();
			
			AST::Node<AST::Type> parseTypeWithQualifier(const Debug::SourcePosition& start,
			                                            Token::Kind qualifier);
			
			AST::Node<AST::Type> parseFunctionPointerType();
			
			AST::Node<AST::TypeList> parseTypeList();
			
			AST::Node<AST::Type> parseBasicType();
			
			AST::Node<AST::Type> parseLongIntOrFloatType(const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> parseIntegerTypeWithSignedness(const Debug::SourcePosition& start,
			                                                    bool isSigned);
			
			AST::Node<AST::Type> parseLongIntegerType(const Debug::SourcePosition& start,
			                                          bool isSigned);
			
			bool isTypeStartToken(Token::Kind kind);
			
		private:
			TokenReader& reader_;
			TypeBuilder builder_;
			
		};
		
	}
	
}

#endif
