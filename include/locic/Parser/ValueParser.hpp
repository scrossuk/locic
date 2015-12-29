#ifndef LOCIC_PARSER_VALUEPARSER_HPP
#define LOCIC_PARSER_VALUEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/ValueBuilder.hpp>

namespace locic {
	
	class PrimitiveID;
	class StringHost;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class ValueParser {
		public:
			ValueParser(TokenReader& reader);
			~ValueParser();
			
			void issueError(Diag diag, const Debug::SourcePosition& start);
			
			bool isValueStartToken(Token::Kind kind) const;
			
			AST::Node<AST::Value> parseValue();
			
			AST::Node<AST::Value> parseTernaryValue();
			
			AST::Node<AST::Value> parseLogicalOrValue();
			
			AST::Node<AST::Value> parseLogicalAndValue();
			
			AST::Node<AST::Value> parseBitwiseOrValue();
			
			AST::Node<AST::Value> parseBitwiseXorValue();
			
			AST::Node<AST::Value> parseBitwiseAndValue();
			
			AST::Node<AST::Value> parseComparisonValue();
			
			AST::Node<AST::Value> parseShiftValue();
			
			AST::Node<AST::Value> parseAddOperatorValue();
			
			AST::Node<AST::Value> parseMultiplyOperatorValue();
			
			AST::Node<AST::Value> parseUnaryValue();
			
			AST::Node<AST::Value> parseCallValue();
			
			AST::Node<AST::Value> parseMemberAccessExpression(AST::Node<AST::Value> value,
			                                                  bool isDeref,
			                                                  const Debug::SourcePosition& start);
			
			String parseMethodName(const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseAtomicValue();
			
			AST::Node<AST::Value> parseAtExpression(const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseSymbolOrLiteralValue();
			
			AST::Node<AST::Value> parseLiteral(Constant constant,
			                                   const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseTypeQualifyingValue(Token::Kind kind,
			                                               const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseArrayLiteral(const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseSymbolSuffix(String firstName,
			                                        const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueList> parseValueList();
			
		private:
			TokenReader& reader_;
			ValueBuilder builder_;
			
		};
		
	}
	
}

#endif
