#ifndef LOCIC_PARSER_VALUEPARSER_HPP
#define LOCIC_PARSER_VALUEPARSER_HPP

#include <locic/AST.hpp>
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
				IN_TYPEDECL,
				
				// If we're in the second value of a ternary
				// expression (i.e. (A ? B : C); we're in B)
				// then don't read the ':' as a type capability
				// test expression.
				IN_TERNARY
			};
			
			ValueParser(TokenReader& reader);
			~ValueParser();
			
			bool isValueStartToken(Token::Kind kind) const;
			
			AST::Node<AST::Value> parseValue(Context context = GENERIC);
			
			AST::Node<AST::Value> parseTernaryValue(Context context);
			
			AST::Node<AST::Value> parseLogicalOrValue(Context context);
			
			void checkLogicalOrOperand(const AST::Node<AST::Value>& operand);
			
			bool isLogicalOrValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseLogicalAndValue(Context context);
			
			void checkLogicalAndOperand(const AST::Node<AST::Value>& operand);
			
			bool isLogicalAndValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseBitwiseOrValue(Context context);
			
			void checkBitwiseOrOperand(const AST::Node<AST::Value>& operand);
			
			bool isBitwiseOrValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseBitwiseXorValue(Context context);
			
			void checkBitwiseXorOperand(const AST::Node<AST::Value>& operand);
			
			bool isBitwiseXorValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseBitwiseAndValue(Context context);
			
			void checkBitwiseAndOperand(const AST::Node<AST::Value>& operand);
			
			bool isBitwiseAndValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseComparisonValue(Context context);
			
			void checkComparisonOperand(const AST::Node<AST::Value>& operand);
			
			bool isComparisonValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseShiftValue(Context context);
			
			void checkShiftOperand(const AST::Node<AST::Value>& operand);
			
			AST::Node<AST::Value> parseAddOperatorValue(Context context);
			
			void checkAddOperand(const AST::Node<AST::Value>& operand);
			
			bool isAddValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseMultiplyOperatorValue(Context context);
			
			void checkMultiplyOperand(const AST::Node<AST::Value>& operand);
			
			bool isMultiplyValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseUnaryValue(Context context);
			
			bool isUnaryValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseCallValue(Context context);
			
			bool isCallValueOrNext(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseMemberAccessExpression(AST::Node<AST::Value> value,
			                                                  bool isDeref,
			                                                  const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseTypeValue(Context context);
			
			bool canInterpretValueAsType(const AST::Node<AST::Value>& value);
			
			AST::Node<AST::Type> interpretValueAsType(AST::Node<AST::Value> value);
			
			AST::Node<AST::Value> parseAtomicValue();
			
			bool isAtomicValue(const AST::Node<AST::Value>& operand) const;
			
			AST::Node<AST::Value> parseAtExpression(const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueList> parseOptionalTemplateArguments();
			
			AST::Node<AST::Value> parseSymbolOrLiteralValue();
			
			AST::Node<AST::Value> parseLiteral(Constant constant,
			                                   const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseTypeQualifyingValue(Token::Kind kind,
			                                               const Debug::SourcePosition& start);
			
			AST::Node<AST::Value> parseArrayLiteral(const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueList> parseValueList(Context context = GENERIC);
			
			AST::Node<AST::Value> parseCastValue();
			
		private:
			TokenReader& reader_;
			ValueBuilder builder_;
			
		};
		
	}
	
}

#endif
