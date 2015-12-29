#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeParser.hpp>
#include <locic/Parser/ValueBuilder.hpp>
#include <locic/Parser/ValueParser.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		ValueParser::ValueParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		ValueParser::~ValueParser() { }
		
		AST::Node<AST::Value> ValueParser::parseValue() {
			return parseTernaryValue();
		}
		
		AST::Node<AST::Value> ValueParser::parseTernaryValue() {
			const auto start = reader_.position();
			
			const auto value = parseLogicalOrValue();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::QUESTIONMARK:
					break;
				default:
					return value;
			}
			
			reader_.consume();
			
			const auto ifTrueValue = parseValue();
			reader_.expect(Token::COLON);
			const auto ifFalseValue = parseTernaryValue();
			return builder_.makeTernaryValue(value, ifTrueValue,
			                                 ifFalseValue, start);
		}
		
		AST::Node<AST::Value> ValueParser::parseLogicalOrValue() {
			const auto start = reader_.position();
			
			auto value = parseLogicalAndValue();
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::DOUBLE_VERTICAL_BAR:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseLogicalAndValue();
				value = builder_.makeLogicalOrValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseLogicalAndValue() {
			const auto start = reader_.position();
			
			auto value = parseBitwiseOrValue();
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::DOUBLE_AMPERSAND:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseBitwiseOrValue();
				value = builder_.makeLogicalAndValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseBitwiseOrValue() {
			const auto start = reader_.position();
			
			auto value = parseBitwiseXorValue();
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::VERTICAL_BAR:
					case Token::OR:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseBitwiseXorValue();
				value = builder_.makeBitwiseOrValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseBitwiseXorValue() {
			const auto start = reader_.position();
			
			auto value = parseBitwiseAndValue();
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::CARET:
					case Token::XOR:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseBitwiseAndValue();
				value = builder_.makeBitwiseXorValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseBitwiseAndValue() {
			const auto start = reader_.position();
			
			auto value = parseComparisonValue();
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::AMPERSAND:
					case Token::AND:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseComparisonValue();
				value = builder_.makeBitwiseAndValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseComparisonValue() {
			const auto start = reader_.position();
			
			auto value = parseShiftValue();
			
			while (true) {
				AST::BinaryOpKind opKind;
				
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::ISEQUAL:
						opKind = AST::OP_ISEQUAL;
						break;
					case Token::NOTEQUAL:
						opKind = AST::OP_NOTEQUAL;
						break;
					case Token::LTRIBRACKET:
						opKind = AST::OP_LESSTHAN;
						break;
					case Token::RTRIBRACKET:
						opKind = AST::OP_GREATERTHAN;
						break;
					case Token::LESSOREQUAL:
						opKind = AST::OP_LESSTHANOREQUAL;
						break;
					case Token::GREATEROREQUAL:
						opKind = AST::OP_GREATERTHANOREQUAL;
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseShiftValue();
				value = builder_.makeBinaryOpValue(value, operand,
				                                   opKind, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseShiftValue() {
			const auto start = reader_.position();
			
			auto value = parseAddOperatorValue();
			
			while (true) {
				AST::BinaryOpKind opKind;
				
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::DOUBLE_LTRIBRACKET:
						reader_.consume();
						opKind = AST::OP_LEFTSHIFT;
						break;
					case Token::RTRIBRACKET:
						reader_.consume();
						reader_.expect(Token::RTRIBRACKET);
						opKind = AST::OP_RIGHTSHIFT;
						break;
					default:
						return value;
				}
				
				const auto operand = parseAddOperatorValue();
				value = builder_.makeBinaryOpValue(value, operand,
				                                   opKind, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseAddOperatorValue() {
			const auto start = reader_.position();
			
			auto value = parseMultiplyOperatorValue();
			
			while (true) {
				AST::BinaryOpKind opKind;
				
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::PLUS:
						opKind = AST::OP_ADD;
						break;
					case Token::MINUS:
						opKind = AST::OP_SUBTRACT;
						break;
					default:
						return value;
				}
				
				reader_.consume();
				const auto operand = parseMultiplyOperatorValue();
				value = builder_.makeBinaryOpValue(value, operand,
				                                   opKind, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseMultiplyOperatorValue() {
			const auto start = reader_.position();
			
			auto value = parseUnaryValue();
			
			while (true) {
				AST::BinaryOpKind opKind;
				
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::STAR:
						opKind = AST::OP_MULTIPLY;
						break;
					case Token::FORWARDSLASH:
						opKind = AST::OP_DIVIDE;
						break;
					case Token::PERCENT:
						opKind = AST::OP_MODULO;
						break;
					default:
						return value;
				}
				
				reader_.consume();
				const auto operand = parseUnaryValue();
				value = builder_.makeBinaryOpValue(value, operand,
				                                   opKind, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseUnaryValue() {
			const auto start = reader_.position();
			
			const auto token = reader_.peek();
			
			AST::UnaryOpKind opKind;
			
			switch (token.kind()) {
				case Token::PLUS:
					opKind = AST::OP_PLUS;
					break;
				case Token::MINUS:
					opKind = AST::OP_MINUS;
					break;
				case Token::EXCLAIMMARK:
					opKind = AST::OP_NOT;
					break;
				case Token::AMPERSAND:
					opKind = AST::OP_ADDRESS;
					break;
				case Token::STAR:
					opKind = AST::OP_DEREF;
					break;
				case Token::MOVE:
					opKind = AST::OP_MOVE;
					break;
				default:
					return parseCallValue();
			}
			
			reader_.consume();
			
			const auto value = parseUnaryValue();
			return builder_.makeUnaryOpValue(value, opKind, start);
		}
		
		AST::Node<AST::Value> ValueParser::parseCallValue() {
			const auto start = reader_.position();
			
			auto value = parseAtomicValue();
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::DOT:
					case Token::PTRACCESS: {
						reader_.consume();
						const bool isDeref = (token.kind() == Token::PTRACCESS);
						value = parseMemberAccessExpression(value, isDeref,
						                                    start);
						break;
					}
					case Token::LROUNDBRACKET: {
						reader_.consume();
						const auto valueList = parseValueList();
						reader_.expect(Token::RROUNDBRACKET);
						value = builder_.makeCallValue(value, valueList, start);
						break;
					}
					case Token::LSQUAREBRACKET: {
						reader_.consume();
						const auto indexValue = parseValue();
						reader_.expect(Token::RSQUAREBRACKET);
						value = builder_.makeIndexValue(value, indexValue, start);
						break;
					}
					default:
						return value;
				}
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseMemberAccessExpression(AST::Node<AST::Value> /*value*/,
		                                                               const bool /*isDeref*/,
		                                                               const Debug::SourcePosition& start) {
			const auto methodName = parseMethodName(start);
			//const auto templateArgs = parseOptionalTemplateArguments(start);
			
			(void) methodName;
			throw std::logic_error("TODO");
		}
		
		String ValueParser::parseMethodName(const Debug::SourcePosition& /*start*/) {
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::NAME:
					reader_.consume();
					return token.name();
				case Token::MOVE:
					reader_.consume();
// 					return stringHost_.getCString("move");
					throw std::logic_error("TODO");
				case Token::NULLVAL:
					reader_.consume();
// 					return stringHost_.getCString("null");
					throw std::logic_error("TODO");
				default:
					throw std::logic_error("TODO");
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseAtomicValue() {
			const auto start = reader_.position();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::LROUNDBRACKET: {
					reader_.consume();
					const auto value = parseValue();
					reader_.expect(Token::RROUNDBRACKET);
					return value;
				}
				case Token::AT:
					reader_.consume();
					return parseAtExpression(start);
				case Token::NAME:
					reader_.consume();
					return parseSymbolOrLiteral(token.name(), start);
				case Token::CONSTANT:
					reader_.consume();
					return parseLiteral(token.constant(), start);
				case Token::REF:
				case Token::LVAL:
					reader_.consume();
					return parseTypeQualifyingValue(token.kind(), start);
				case Token::SELF:
					return builder_.makeSelfValue(start);
				case Token::THIS:
					return builder_.makeThisValue(start);
				case Token::ALIGNOF: {
					reader_.consume();
					const auto type = TypeParser(reader_).parseType();
					return builder_.makeAlignOfValue(type, start);
				}
				case Token::SIZEOF: {
					reader_.consume();
					const auto type = TypeParser(reader_).parseType();
					return builder_.makeSizeOfValue(type, start);
				}
				case Token::LCURLYBRACKET:
					reader_.consume();
					return parseArrayLiteral(start);
				default:
					throw std::logic_error("TODO");
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseAtExpression(const Debug::SourcePosition& start) {
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::LROUNDBRACKET: {
					throw std::logic_error("TODO");
				}
				case Token::LTRIBRACKET: {
					throw std::logic_error("TODO");
				}
				case Token::NAME: {
					reader_.consume();
					return builder_.makeSelfMemberAccess(token.name(), start);
				}
				default:
					throw std::logic_error("TODO");
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseSymbolOrLiteral(const String name,
		                                                        const Debug::SourcePosition& start) {
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::CONSTANT: {
					reader_.consume();
					return builder_.makeLiteralValue(token.constant(),
					                                 name, start);
				}
				default:
					return parseSymbolSuffix(name, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseLiteral(const Constant constant,
		                                                const Debug::SourcePosition& start) {
			String specifier;
			
			const auto token = reader_.peek();
			if (token.kind() == Token::NAME) {
				reader_.consume();
				specifier = token.name();
			}
			
			return builder_.makeLiteralValue(constant, specifier, start);
		}
		
		AST::Node<AST::Value> ValueParser::parseTypeQualifyingValue(const Token::Kind /*kind*/,
		                                                            const Debug::SourcePosition& /*start*/) {
			throw std::logic_error("TODO");
		}
		
		AST::Node<AST::Value> ValueParser::parseArrayLiteral(const Debug::SourcePosition& start) {
			const auto valueList = parseValueList();
			reader_.expect(Token::RCURLYBRACKET);
			return builder_.makeArrayLiteralValue(valueList, start);
		}
		
		AST::Node<AST::Value> ValueParser::parseSymbolSuffix(String /*firstName*/,
		                                                     const Debug::SourcePosition& /*start*/) {
			throw std::logic_error("TODO");
		}
		
		AST::Node<AST::ValueList> ValueParser::parseValueList() {
			const auto start = reader_.position();
			
			AST::ValueList valueList;
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::RCURLYBRACKET:
					case Token::RTRIBRACKET:
					case Token::RROUNDBRACKET:
						return builder_.makeValueList(valueList, start);
					default:
						break;
				}
				
				valueList.push_back(parseValue());
				
				if (reader_.peek().kind() != Token::COMMA) {
					return builder_.makeValueList(valueList, start);
				}
				
				reader_.consume();
			}
		}
		
	}
	
}
