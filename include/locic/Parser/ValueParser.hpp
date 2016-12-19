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
			
			AST::Node<AST::ValueDecl> parseValue(Context context = GENERIC);
			
			AST::Node<AST::ValueDecl> parseTernaryValue(Context context);
			
			AST::Node<AST::ValueDecl> parseLogicalOrValue(Context context);
			
			void checkLogicalOrOperand(const AST::Node<AST::ValueDecl>& operand);
			
			bool isLogicalOrValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseLogicalAndValue(Context context);
			
			void checkLogicalAndOperand(const AST::Node<AST::ValueDecl>& operand);
			
			bool isLogicalAndValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseBitwiseOrValue(Context context);
			
			void checkBitwiseOrOperand(const AST::Node<AST::ValueDecl>& operand);
			
			bool isBitwiseOrValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseBitwiseXorValue(Context context);
			
			void checkBitwiseXorOperand(const AST::Node<AST::ValueDecl>& operand);
			
			bool isBitwiseXorValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseBitwiseAndValue(Context context);
			
			void checkBitwiseAndOperand(const AST::Node<AST::ValueDecl>& operand);
			
			bool isBitwiseAndValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseComparisonValue(Context context);
			
			void checkComparisonOperand(const AST::Node<AST::ValueDecl>& operand);
			
			bool isComparisonValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseShiftValue(Context context);
			
			void checkShiftOperand(const AST::Node<AST::ValueDecl>& operand);
			
			AST::Node<AST::ValueDecl> parseAddOperatorValue(Context context);
			
			void checkAddOperand(const AST::Node<AST::ValueDecl>& operand);
			
			bool isAddValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseMultiplyOperatorValue(Context context);
			
			void checkMultiplyOperand(const AST::Node<AST::ValueDecl>& operand);
			
			bool isMultiplyValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseUnaryValue(Context context);
			
			bool isUnaryValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseCallValue(Context context);
			
			bool isCallValueOrNext(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseMemberAccessExpression(AST::Node<AST::ValueDecl> value,
			                                                  bool isDeref,
			                                                  const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl> parseTypeValue(Context context);
			
			bool canInterpretValueAsType(const AST::Node<AST::ValueDecl>& value);
			
			AST::Node<AST::TypeDecl> interpretValueAsType(AST::Node<AST::ValueDecl> value);
			
			AST::Node<AST::ValueDecl> parseAtomicValue();
			
			bool isAtomicValue(const AST::Node<AST::ValueDecl>& operand) const;
			
			AST::Node<AST::ValueDecl> parseAtExpression(const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDeclList> parseOptionalTemplateArguments();
			
			AST::Node<AST::ValueDecl> parseSymbolOrLiteralValue();
			
			AST::Node<AST::ValueDecl> parseLiteral(Constant constant,
			                                   const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl> parseTypeQualifyingValue(Token::Kind kind,
			                                               const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl> parseArrayLiteral(const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDeclList> parseValueList(Context context = GENERIC);
			
			AST::Node<AST::ValueDecl> parseCastValue();
			
		private:
			TokenReader& reader_;
			ValueBuilder builder_;
			
		};
		
	}
	
}

#endif
