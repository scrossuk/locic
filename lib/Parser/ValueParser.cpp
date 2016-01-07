#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/FunctionParser.hpp>
#include <locic/Parser/SymbolParser.hpp>
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
		
		bool ValueParser::isValueStartToken(const Token::Kind kind) const {
			switch (kind) {
				case Token::NAME:
				case Token::PLUS:
				case Token::MINUS:
				case Token::EXCLAIMMARK:
				case Token::AMPERSAND:
				case Token::STAR:
				case Token::TILDA:
				case Token::CONSTANT:
				case Token::LROUNDBRACKET:
				case Token::LCURLYBRACKET:
				case Token::MOVE:
				case Token::NULLVAL:
				case Token::LVAL:
				case Token::NOLVAL:
				case Token::REF:
				case Token::NOREF:
				case Token::STATICREF:
				case Token::NOTAG:
				case Token::AT:
				case Token::SELF:
				case Token::THIS:
				case Token::ALIGNOF:
				case Token::SIZEOF:
				case Token::TYPEOF:
				case Token::TYPEID:
				case Token::TRUEVAL:
				case Token::FALSEVAL:
				case Token::CONST_CAST:
				case Token::DYNAMIC_CAST:
				case Token::REINTERPRET_CAST:
				case Token::IS_A:
				case Token::TYPENAME:
				case Token::VOID:
				case Token::BOOL:
				case Token::BYTE:
				case Token::UBYTE:
				case Token::SHORT:
				case Token::USHORT:
				case Token::INT:
				case Token::UINT:
				case Token::LONG:
				case Token::ULONG:
				case Token::LONGLONG:
				case Token::ULONGLONG:
				case Token::FLOAT:
				case Token::DOUBLE:
				case Token::CONST:
				case Token::SIGNED:
				case Token::UNSIGNED:
					return true;
				// Specify all the 'false' cases so that any new
				// tokens will have to be carefully added here.
				case Token::FINAL:
				case Token::MUTABLE:
				case Token::DOUBLE_COLON:
				case Token::DOUBLE_AMPERSAND:
				case Token::DOUBLE_VERTICAL_BAR:
				case Token::DOUBLE_PLUS:
				case Token::DOUBLE_MINUS:
				case Token::DOUBLE_LTRIBRACKET:
				case Token::LTRIBRACKET:
				case Token::RTRIBRACKET:
				case Token::LSQUAREBRACKET:
				case Token::RSQUAREBRACKET:
				case Token::RROUNDBRACKET:
				case Token::RCURLYBRACKET:
				case Token::UNUSED:
				case Token::UNUSED_RESULT:
				case Token::UNDERSCORE:
				case Token::LET:
				case Token::VERSION:
				case Token::SEMICOLON:
				case Token::COLON:
				case Token::QUESTIONMARK:
				case Token::CLASS:
				case Token::DATATYPE:
				case Token::ENUM:
				case Token::EXCEPTION:
				case Token::INTERFACE:
				case Token::NAMESPACE:
				case Token::STRUCT:
				case Token::UNION:
				case Token::TEMPLATE:
				case Token::IMPORT:
				case Token::EXPORT:
				case Token::AUTO:
				case Token::STATIC:
				case Token::REQUIRE:
				case Token::VIRTUAL:
				case Token::USING:
				case Token::CASE:
				case Token::SWITCH:
				case Token::IF:
				case Token::ELSE:
				case Token::WHILE:
				case Token::FOR:
				case Token::DEFAULT:
				case Token::CONTINUE:
				case Token::BREAK:
				case Token::ASSERT:
				case Token::RETURN:
				case Token::UNREACHABLE:
				case Token::THROW:
				case Token::TRY:
				case Token::CATCH:
				case Token::SCOPE:
				case Token::NOEXCEPT:
				case Token::PRIMITIVE:
				case Token::PRIMITIVEFUNCTION:
				case Token::OVERRIDE_CONST:
				case Token::COMMA:
				case Token::SETEQUAL:
				case Token::ADDEQUAL:
				case Token::SUBEQUAL:
				case Token::MULEQUAL:
				case Token::DIVEQUAL:
				case Token::PERCENTEQUAL:
				case Token::AND:
				case Token::OR:
				case Token::XOR:
				case Token::DOT:
				case Token::PTRACCESS:
				case Token::VERTICAL_BAR:
				case Token::FORWARDSLASH:
				case Token::PERCENT:
				case Token::ISEQUAL:
				case Token::NOTEQUAL:
				case Token::GREATEROREQUAL:
				case Token::LESSOREQUAL:
				case Token::CARET:
				case Token::UNKNOWN:
				case Token::ERROR:
				case Token::END:
					return false;
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseValue(const Context context) {
			return parseTernaryValue(context);
		}
		
		AST::Node<AST::Value> ValueParser::parseTernaryValue(const Context context) {
			const auto start = reader_.position();
			
			const auto value = parseLogicalOrValue(context);
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::QUESTIONMARK:
					break;
				default:
					return value;
			}
			
			reader_.consume();
			
			const auto ifTrueValue = parseValue(IN_TERNARY);
			reader_.expect(Token::COLON);
			const auto ifFalseValue = parseTernaryValue(context);
			return builder_.makeTernaryValue(value, ifTrueValue,
			                                 ifFalseValue, start);
		}
		
		AST::Node<AST::Value> ValueParser::parseLogicalOrValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseLogicalAndValue(context);
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::DOUBLE_VERTICAL_BAR:
					case Token::OR:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseLogicalAndValue(context);
				value = builder_.makeLogicalOrValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseLogicalAndValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseBitwiseOrValue(context);
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::DOUBLE_AMPERSAND:
					case Token::AND:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseBitwiseOrValue(context);
				value = builder_.makeLogicalAndValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseBitwiseOrValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseBitwiseXorValue(context);
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::VERTICAL_BAR:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseBitwiseXorValue(context);
				value = builder_.makeBitwiseOrValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseBitwiseXorValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseBitwiseAndValue(context);
			
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
				
				const auto operand = parseBitwiseAndValue(context);
				value = builder_.makeBitwiseXorValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseBitwiseAndValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseComparisonValue(context);
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::AMPERSAND:
						break;
					default:
						return value;
				}
				
				reader_.consume();
				
				const auto operand = parseComparisonValue(context);
				value = builder_.makeBitwiseAndValue(value, operand, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseComparisonValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseShiftValue(context);
			
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
						if (context == IN_TEMPLATE) {
							// Can't have '<' in template values.
							return value;
						}
						opKind = AST::OP_LESSTHAN;
						break;
					case Token::RTRIBRACKET:
						if (context == IN_TEMPLATE) {
							// Can't have '>' in template values.
							return value;
						}
						opKind = AST::OP_GREATERTHAN;
						break;
					case Token::LESSOREQUAL:
						opKind = AST::OP_LESSTHANOREQUAL;
						break;
					case Token::GREATEROREQUAL:
						opKind = AST::OP_GREATERTHANOREQUAL;
						break;
					case Token::COLON: {
						if (context == IN_TERNARY) {
							return value;
						}
						
						const auto checkType = interpretValueAsType(value);
						reader_.consume();
						const auto capabilityType = TypeParser(reader_).parseType();
						
						value = builder_.makeCapabilityTest(checkType,
						                                    capabilityType,
						                                    start);
						break;
					}
					default:
						return value;
				}
				
				if (token.kind() == Token::COLON) {
					continue;
				}
				
				reader_.consume();
				
				const auto operand = parseShiftValue(context);
				value = builder_.makeBinaryOpValue(value, operand,
				                                   opKind, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseShiftValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseAddOperatorValue(context);
			
			while (true) {
				AST::BinaryOpKind opKind;
				
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::DOUBLE_LTRIBRACKET:
						reader_.consume();
						opKind = AST::OP_LEFTSHIFT;
						break;
					case Token::RTRIBRACKET:
						if (context == IN_TEMPLATE) {
							// Can't have '>>' in template values.
							return value;
						}
						if (reader_.peek(/*offset=*/1).kind() != Token::RTRIBRACKET) {
							return value;
						}
						reader_.consume();
						reader_.expect(Token::RTRIBRACKET);
						opKind = AST::OP_RIGHTSHIFT;
						break;
					default:
						return value;
				}
				
				const auto operand = parseAddOperatorValue(context);
				value = builder_.makeBinaryOpValue(value, operand,
				                                   opKind, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseAddOperatorValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseMultiplyOperatorValue(context);
			
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
				const auto operand = parseMultiplyOperatorValue(context);
				value = builder_.makeBinaryOpValue(value, operand,
				                                   opKind, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseMultiplyOperatorValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseUnaryValue(context);
			
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
				const auto operand = parseUnaryValue(context);
				value = builder_.makeBinaryOpValue(value, operand,
				                                   opKind, start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseUnaryValue(const Context context) {
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
					return parseCallValue(context);
			}
			
			reader_.consume();
			
			const auto value = parseUnaryValue(context);
			return builder_.makeUnaryOpValue(value, opKind, start);
		}
		
		AST::Node<AST::Value> ValueParser::parseCallValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseTypeValue(context);
			
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
		
		AST::Node<AST::Value> ValueParser::parseMemberAccessExpression(AST::Node<AST::Value> value,
		                                                               const bool isDeref,
		                                                               const Debug::SourcePosition& start) {
			const auto memberName = FunctionParser(reader_).parseFunctionNameElement();
			const auto templateArgs = SymbolParser(reader_).parseSymbolTemplateArgumentList();
			
			if (isDeref) {
				value = builder_.makeDerefValue(value, start);
			}
			
			if (templateArgs->empty()) {
				return builder_.makeMemberAccess(value, memberName, start);
			} else {
				return builder_.makeTemplatedMemberAccess(value, memberName,
				                                          templateArgs,
				                                          start);
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseTypeValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseAtomicValue();
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::STAR:
					case Token::AMPERSAND: {
						const auto secondToken = reader_.peek(/*offset=*/1);
						if (context != IN_TYPEDECL &&
						    isValueStartToken(secondToken.kind())) {
							// Next token is a value, so this is a
							// multiply/bitwise-AND value.
							return value;
						}
						
						auto type = interpretValueAsType(value);
						if (token.kind() == Token::STAR) {
							type = TypeBuilder(reader_).makePointerType(type, start);
						} else {
							type = TypeBuilder(reader_).makeReferenceType(type, start);
						}
						
						reader_.consume();
						
						value = builder_.makeTypeValue(type, start);
						break;
					}
					case Token::LSQUAREBRACKET: {
						if (!canInterpretValueAsType(value)) {
							return value;
						}
						
						reader_.consume();
						const auto indexValue = parseValue();
						reader_.expect(Token::RSQUAREBRACKET);
						
						const auto type = interpretValueAsType(value);
						const auto staticArrayType =
						    TypeBuilder(reader_).makeStaticArrayType(type, indexValue,
						                                             start);
						const auto typeValue = builder_.makeTypeValue(staticArrayType, start);
						
						if (value->kind() == AST::Value::TYPEREF) {
							// Definitely a type.
							value = typeValue;
							break;
						}
						
						// Ambiguous: could be indexing or a static array type.
						const auto indexResult = builder_.makeIndexValue(value, indexValue, start);
						value = builder_.makeMergeValue(indexResult, typeValue, start);
						break;
					}
					default:
						return value;
				}
			}
		}
		
		bool ValueParser::canInterpretValueAsType(const AST::Node<AST::Value>& value) {
			switch (value->kind()) {
				case AST::Value::BRACKET:
					return canInterpretValueAsType(value->bracket.value);
				case AST::Value::SYMBOLREF:
				case AST::Value::TYPEREF:
					return true;
				case AST::Value::MERGE:
					return canInterpretValueAsType(value->merge.first) ||
					       canInterpretValueAsType(value->merge.second);
				case AST::Value::SELF:
				case AST::Value::THIS:
				case AST::Value::LITERAL:
				case AST::Value::MEMBERREF:
				case AST::Value::ALIGNOF:
				case AST::Value::SIZEOF:
				case AST::Value::UNARYOP:
				case AST::Value::BINARYOP:
				case AST::Value::TERNARY:
				case AST::Value::CAST:
				case AST::Value::LVAL:
				case AST::Value::NOLVAL:
				case AST::Value::REF:
				case AST::Value::NOREF:
				case AST::Value::INTERNALCONSTRUCT:
				case AST::Value::MEMBERACCESS:
				case AST::Value::TEMPLATEDMEMBERACCESS:
				case AST::Value::FUNCTIONCALL:
				case AST::Value::CAPABILITYTEST:
				case AST::Value::ARRAYLITERAL:
					return false;
			}
		}
		
		AST::Node<AST::Type> ValueParser::interpretValueAsType(const AST::Node<AST::Value>& value) {
			switch (value->kind()) {
				case AST::Value::BRACKET:
					return interpretValueAsType(value->bracket.value);
				case AST::Value::SYMBOLREF:
					return AST::makeNode(value.location(),
					                     AST::Type::Object(value->symbolRef.symbol));
				case AST::Value::TYPEREF:
					return value->typeRef.type;
				case AST::Value::MERGE:
					if (canInterpretValueAsType(value->merge.second)) {
						return interpretValueAsType(value->merge.second);
					} else {
						return interpretValueAsType(value->merge.first);
					}
				case AST::Value::SELF:
				case AST::Value::THIS:
				case AST::Value::LITERAL:
				case AST::Value::MEMBERREF:
				case AST::Value::ALIGNOF:
				case AST::Value::SIZEOF:
				case AST::Value::UNARYOP:
				case AST::Value::BINARYOP:
				case AST::Value::TERNARY:
				case AST::Value::CAST:
				case AST::Value::LVAL:
				case AST::Value::NOLVAL:
				case AST::Value::REF:
				case AST::Value::NOREF:
				case AST::Value::INTERNALCONSTRUCT:
				case AST::Value::MEMBERACCESS:
				case AST::Value::TEMPLATEDMEMBERACCESS:
				case AST::Value::FUNCTIONCALL:
				case AST::Value::CAPABILITYTEST:
				case AST::Value::ARRAYLITERAL: {
					throw std::logic_error("TODO: Invalid value to be interpreted as type");
				}
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
					return parseSymbolOrLiteralValue();
				case Token::NULLVAL:
					reader_.consume();
					return builder_.makeLiteralValue(Constant::Null(),
					                                 reader_.makeCString(""),
					                                 start);
				case Token::TRUEVAL:
					reader_.consume();
					return builder_.makeLiteralValue(Constant::True(),
					                                 reader_.makeCString(""),
					                                 start);
				case Token::FALSEVAL:
					reader_.consume();
					return builder_.makeLiteralValue(Constant::False(),
					                                 reader_.makeCString(""),
					                                 start);
				case Token::CONSTANT:
					reader_.consume();
					return parseLiteral(token.constant(), start);
				case Token::REF:
				case Token::NOREF:
				case Token::LVAL:
				case Token::NOLVAL:
					reader_.consume();
					return parseTypeQualifyingValue(token.kind(), start);
				case Token::SELF:
					reader_.consume();
					return builder_.makeSelfValue(start);
				case Token::THIS:
					reader_.consume();
					return builder_.makeThisValue(start);
				case Token::ALIGNOF: {
					reader_.consume();
					reader_.expect(Token::LROUNDBRACKET);
					const auto type = TypeParser(reader_).parseType();
					reader_.expect(Token::RROUNDBRACKET);
					return builder_.makeAlignOfValue(type, start);
				}
				case Token::SIZEOF: {
					reader_.consume();
					reader_.expect(Token::LROUNDBRACKET);
					const auto type = TypeParser(reader_).parseType();
					reader_.expect(Token::RROUNDBRACKET);
					return builder_.makeSizeOfValue(type, start);
				}
				case Token::LCURLYBRACKET:
					reader_.consume();
					return parseArrayLiteral(start);
				case Token::CONST_CAST:
				case Token::DYNAMIC_CAST:
				case Token::REINTERPRET_CAST: {
					return parseCastValue();
				}
				default:
					break;
			}
			
			if (TypeParser(reader_).isTypeStartToken(token.kind())) {
				const auto type = TypeParser(reader_).parseType();
				return builder_.makeTypeValue(type, start);
			}
			
			printf("Unexpected: %s\n", token.toString().c_str());
			
			throw std::logic_error("TODO: invalid atomic value");
		}
		
		AST::Node<AST::Value> ValueParser::parseAtExpression(const Debug::SourcePosition& start) {
			const auto token = reader_.peek();
			if (token.kind() == Token::NAME) {
				reader_.consume();
				return builder_.makeSelfMemberAccess(token.name(), start);
			}
			
			const auto templateArguments = parseOptionalTemplateArguments();
			
			reader_.expect(Token::LROUNDBRACKET);
			const auto arguments = parseValueList();
			reader_.expect(Token::RROUNDBRACKET);
			return builder_.makeInternalConstruct(templateArguments,
			                                      arguments, start);
		}
		
		AST::Node<AST::ValueList> ValueParser::parseOptionalTemplateArguments() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::LTRIBRACKET) {
				return builder_.makeValueList(AST::ValueList(), start);
			}
			
			reader_.consume();
			const auto valueList = parseValueList(IN_TEMPLATE);
			reader_.expect(Token::RTRIBRACKET);
			
			return valueList;
		}
		
		AST::Node<AST::Value> ValueParser::parseSymbolOrLiteralValue() {
			const auto start = reader_.position();
			
			const auto firstToken = reader_.peek(/*offset=*/0);
			assert(firstToken.kind() == Token::NAME);
			
			const auto secondToken = reader_.peek(/*offset=*/1);
			
			switch (secondToken.kind()) {
				case Token::CONSTANT: {
					// This is a literal specifier in prefix form
					// (i.e. NAME then CONSTANT).
					reader_.consume();
					reader_.consume();
					return builder_.makeLiteralValue(secondToken.constant(),
					                                 firstToken.name(), start);
				}
				default: {
					const auto symbol = SymbolParser(reader_).parseSymbol();
					return builder_.makeSymbolValue(symbol, start);
				}
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseLiteral(const Constant constant,
		                                                const Debug::SourcePosition& start) {
			String specifier;
			
			const auto token = reader_.peek();
			if (token.kind() == Token::NAME) {
				reader_.consume();
				specifier = token.name();
			} else {
				specifier = reader_.makeCString("");
			}
			
			return builder_.makeLiteralValue(constant, specifier, start);
		}
		
		AST::Node<AST::Value> ValueParser::parseTypeQualifyingValue(const Token::Kind kind,
		                                                            const Debug::SourcePosition& start) {
			switch (kind) {
				case Token::REF:
				case Token::LVAL: {
					reader_.expect(Token::LTRIBRACKET);
					const auto targetType = TypeParser(reader_).parseType();
					reader_.expect(Token::RTRIBRACKET);
					reader_.expect(Token::LROUNDBRACKET);
					const auto value = parseValue();
					reader_.expect(Token::RROUNDBRACKET);
					if (kind == Token::REF) {
						return builder_.makeRefValue(targetType, value, start);
					} else {
						return builder_.makeLvalValue(targetType, value, start);
					}
				}
				default: {
					reader_.expect(Token::LROUNDBRACKET);
					const auto value = parseValue();
					reader_.expect(Token::RROUNDBRACKET);
					if (kind == Token::NOREF) {
						return builder_.makeNoRefValue(value, start);
					} else {
						return builder_.makeNoLvalValue(value, start);
					}
				}
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseArrayLiteral(const Debug::SourcePosition& start) {
			const auto valueList = parseValueList();
			reader_.expect(Token::RCURLYBRACKET);
			return builder_.makeArrayLiteralValue(valueList, start);
		}
		
		AST::Node<AST::ValueList> ValueParser::parseValueList(const Context context) {
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
				
				valueList.push_back(parseValue(context));
				
				if (reader_.peek().kind() != Token::COMMA) {
					return builder_.makeValueList(valueList, start);
				}
				
				reader_.consume();
			}
		}
		
		AST::Node<AST::Value> ValueParser::parseCastValue() {
			const auto start = reader_.position();
			
			auto tokens = {
				Token::CONST_CAST,
				Token::DYNAMIC_CAST,
				Token::REINTERPRET_CAST
			};
			
			auto kind = AST::Value::CAST_CONST;
			
			const auto token = reader_.expectOneOf(tokens);
			switch (token.kind()) {
				case Token::CONST_CAST:
					kind = AST::Value::CAST_CONST;
					break;
				case Token::DYNAMIC_CAST:
					kind = AST::Value::CAST_DYNAMIC;
					break;
				case Token::REINTERPRET_CAST:
					kind = AST::Value::CAST_REINTERPRET;
					break;
				default:
					break;
			}
			
			reader_.expect(Token::LTRIBRACKET);
			const auto fromType = TypeParser(reader_).parseType();
			reader_.expect(Token::COMMA);
			const auto toType = TypeParser(reader_).parseType();
			reader_.expect(Token::RTRIBRACKET);
			
			reader_.expect(Token::LROUNDBRACKET);
			const auto value = parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makeCastValue(kind, fromType, toType, value, start);
		}
		
	}
	
}
