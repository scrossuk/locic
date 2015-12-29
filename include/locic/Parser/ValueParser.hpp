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
			enum Context {
				GENERIC,
				
				// If we're in the middle of parsing template
				// arguments, we can't accept comparison (<, >)
				// operations or right shifts (>>).
				IN_TEMPLATE,
				
				// If we're in the middle of a possible type
				// declaration, then 'VAL & NAME' should be
				// treated as a variable definition rather than
				// bitwise AND.
				IN_TYPEDECL
			};
			
			ValueParser(TokenReader& reader);
			~ValueParser();
			
			void issueError(Diag diag, const Debug::SourcePosition& start);
			
			bool isValueStartToken(Token::Kind kind) const;
			
			AST::Node<AST::Value> parseValue(Context context = GENERIC);
			
			AST::Node<AST::Value> parseTernaryValue(Context context);
			
			AST::Node<AST::Value> parseLogicalOrValue(Context context);
			
			AST::Node<AST::Value> parseLogicalAndValue(Context context);
			
			AST::Node<AST::Value> parseBitwiseOrValue(Context context);
			
			AST::Node<AST::Value> parseBitwiseXorValue(Context context);
			
			AST::Node<AST::Value> parseBitwiseAndValue(Context context);
			
			AST::Node<AST::Value> parseComparisonValue(Context context);
			
			AST::Node<AST::Value> parseShiftValue(Context context);
			
			AST::Node<AST::Value> parseAddOperatorValue(Context context);
			
			AST::Node<AST::Value> parseMultiplyOperatorValue(Context context);
			
			AST::Node<AST::Value> parseUnaryValue(Context context);
			
			AST::Node<AST::Value> parseCallValue(Context context);
			
			AST::Node<AST::Value> parseMemberAccessExpression(AST::Node<AST::Value> value,
			                                                  bool isDeref,
			                                                  const Debug::SourcePosition& start);
			
			String parseMethodName(const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseTypeValue(Context context);
			
			AST::Node<AST::Value> parseAtomicValue();
			
			AST::Node<AST::Value> parseAtExpression(const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseSymbolOrLiteralValue();
			
			AST::Node<AST::Value> parseLiteral(Constant constant,
			                                   const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseTypeQualifyingValue(Token::Kind kind,
			                                               const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseArrayLiteral(const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueList> parseValueList();
			
		private:
			TokenReader& reader_;
			ValueBuilder builder_;
			
		};
		
	}
	
}

#endif
