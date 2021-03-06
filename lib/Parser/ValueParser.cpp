#include <locic/AST.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Parser/FunctionParser.hpp>
#include <locic/Parser/SymbolParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeBuilder.hpp>
#include <locic/Parser/TypeParser.hpp>
#include <locic/Parser/ValueBuilder.hpp>
#include <locic/Parser/ValueParser.hpp>
#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		class InvalidValidDiag: public ErrorDiag {
		public:
			InvalidValidDiag(const Token::Kind actual)
			: actual_(actual) { }
			
			std::string toString() const {
				return makeString("unexpected value token: %s",
				                  Token::kindToString(actual_).c_str());
			}
			
		private:
			Token::Kind actual_;
			
		};
		
		class InvalidOperandDiag: public WarningDiag {
		public:
			InvalidOperandDiag(const std::string& opName)
			: opName_(opName) { }
			
			std::string toString() const {
				return makeString("operand of %s depends on subtle precedence; add parentheses",
				                  opName_.c_str());
			}
			
		private:
			std::string opName_;
			
		};
		
		class CannotInterpretValueAsType: public ErrorDiag {
		public:
			CannotInterpretValueAsType()  { }
			
			std::string toString() const {
				return "cannot interpret value as type";
			}
		};
		
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
				case Token::NEW:
				case Token::NULLVAL:
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
				case Token::UNICHAR:
				case Token::CONST:
				case Token::SELFCONST:
				case Token::SIGNED:
				case Token::UNSIGNED:
					return true;
				// Specify all the 'false' cases so that any new
				// tokens will have to be carefully added here.
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
				case Token::VARIANT:
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
				case Token::INHERIT:
				case Token::OVERRIDE:
				case Token::UNKNOWN:
				case Token::ERROR:
				case Token::END:
					return false;
			}
			
			locic_unreachable("Invalid token kind.");
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseValue(const Context context) {
			return parseTernaryValue(context);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseTernaryValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseLogicalOrValue(context);
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::QUESTIONMARK:
					break;
				default:
					return value;
			}
			
			reader_.consume();
			
			auto ifTrueValue = parseValue(IN_TERNARY);
			reader_.expect(Token::COLON);
			auto ifFalseValue = parseTernaryValue(context);
			
			if (!isComparisonValueOrNext(value)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("ternary"), value.location());
			}
			
			return builder_.makeTernaryValue(std::move(value), std::move(ifTrueValue),
			                                 std::move(ifFalseValue), start);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseLogicalOrValue(const Context context) {
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
				
				auto operand = parseLogicalAndValue(context);
				checkLogicalOrOperand(value);
				checkLogicalOrOperand(operand);
				value = builder_.makeLogicalOrValue(std::move(value), std::move(operand), start);
			}
		}
		
		void ValueParser::checkLogicalOrOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isLogicalOrValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("logical or"), operand.location());
			}
		}
		
		bool ValueParser::isLogicalOrValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			return operand->isLogicalOr() || isComparisonValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseLogicalAndValue(const Context context) {
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
				
				auto operand = parseBitwiseOrValue(context);
				checkLogicalAndOperand(value);
				checkLogicalAndOperand(operand);
				value = builder_.makeLogicalAndValue(std::move(value), std::move(operand), start);
			}
		}
		
		void ValueParser::checkLogicalAndOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isLogicalAndValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("logical and"), operand.location());
			}
		}
		
		bool ValueParser::isLogicalAndValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			return operand->isLogicalAnd() || isComparisonValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseBitwiseOrValue(const Context context) {
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
				
				auto operand = parseBitwiseXorValue(context);
				checkBitwiseOrOperand(value);
				checkBitwiseOrOperand(operand);
				value = builder_.makeBitwiseOrValue(std::move(value), std::move(operand), start);
			}
		}
		
		void ValueParser::checkBitwiseOrOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isBitwiseOrValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("bitwise or"), operand.location());
			}
		}
		
		bool ValueParser::isBitwiseOrValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			return operand->isBitwiseOr() || isUnaryValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseBitwiseXorValue(const Context context) {
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
				
				auto operand = parseBitwiseAndValue(context);
				checkBitwiseXorOperand(value);
				checkBitwiseXorOperand(operand);
				value = builder_.makeBitwiseXorValue(std::move(value), std::move(operand), start);
			}
		}
		
		void ValueParser::checkBitwiseXorOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isBitwiseXorValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("bitwise xor"), operand.location());
			}
		}
		
		bool ValueParser::isBitwiseXorValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			return operand->isBitwiseXor() || isUnaryValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseBitwiseAndValue(const Context context) {
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
				
				auto operand = parseComparisonValue(context);
				checkBitwiseAndOperand(value);
				checkBitwiseAndOperand(operand);
				value = builder_.makeBitwiseAndValue(std::move(value), std::move(operand), start);
			}
		}
		
		void ValueParser::checkBitwiseAndOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isBitwiseAndValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("bitwise and"), operand.location());
			}
		}
		
		bool ValueParser::isBitwiseAndValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			return operand->isBitwiseAnd() || isUnaryValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseComparisonValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseShiftValue(context);
			
			while (true) {
				const auto token = reader_.peek();
				const auto kind = token.kind();
				if (kind == Token::COLON) {
					if (context == IN_TERNARY) {
						return value;
					}
					
					auto checkType = interpretValueAsType(std::move(value));
					reader_.consume();
					auto capabilityType = TypeParser(reader_).parseType();
					
					value = builder_.makeCapabilityTest(std::move(checkType),
					                                    std::move(capabilityType),
					                                    start);
					continue;
				}
				
				AST::BinaryOpKind opKind;
				switch (kind) {
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
					default:
						return value;
				}
				
				reader_.consume();
				
				auto operand = parseShiftValue(context);
				checkComparisonOperand(value);
				checkComparisonOperand(operand);
				value = builder_.makeBinaryOpValue(std::move(value), std::move(operand),
				                                   opKind, start);
			}
		}
		
		void ValueParser::checkComparisonOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isAddValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("comparison"), operand.location());
			}
		}
		
		bool ValueParser::isComparisonValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			// Use a unary value as the next level down to avoid conditions like
			// 'a + b' or 'a * b'.
			return operand->isComparison() || operand->isCapabilityTest() ||
			       isUnaryValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseShiftValue(const Context context) {
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
				
				auto operand = parseAddOperatorValue(context);
				checkShiftOperand(value);
				checkShiftOperand(operand);
				value = builder_.makeBinaryOpValue(std::move(value), std::move(operand),
				                                   opKind, start);
			}
		}
		
		void ValueParser::checkShiftOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isUnaryValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("shift"), operand.location());
			}
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseAddOperatorValue(const Context context) {
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
				auto operand = parseMultiplyOperatorValue(context);
				checkAddOperand(value);
				checkAddOperand(operand);
				value = builder_.makeBinaryOpValue(std::move(value), std::move(operand),
				                                   opKind, start);
			}
		}
		
		void ValueParser::checkAddOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isAddValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("add/subtract"), operand.location());
			}
		}
		
		bool ValueParser::isAddValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			return operand->isAdd() || operand->isSubtract() || isMultiplyValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseMultiplyOperatorValue(const Context context) {
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
				auto operand = parseUnaryValue(context);
				checkMultiplyOperand(value);
				checkMultiplyOperand(operand);
				value = builder_.makeBinaryOpValue(std::move(value), std::move(operand),
				                                   opKind, start);
			}
		}
		
		void ValueParser::checkMultiplyOperand(const AST::Node<AST::ValueDecl>& operand) {
			if (!isMultiplyValueOrNext(operand)) {
				reader_.issueDiagWithLoc(InvalidOperandDiag("multiply/subtract/divide"), operand.location());
			}
		}
		
		bool ValueParser::isMultiplyValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			return operand->isMultiply() || operand->isDivide() || operand->isModulo() ||
			       isUnaryValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseUnaryValue(const Context context) {
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
				case Token::NEW:
					return parseNewExpression(context);
				default:
					return parseCallValue(context);
			}
			
			reader_.consume();
			
			auto value = parseUnaryValue(context);
			return builder_.makeUnaryOpValue(std::move(value), opKind, start);
		}
		
		bool ValueParser::isUnaryValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			return operand->isUnaryOp() || isCallValueOrNext(operand);
		}
		
		AST::Node<AST::ValueDecl>
		ValueParser::parseNewExpression(const Context context) {
			const auto start = reader_.position();
			
			reader_.expect(Token::NEW);
			// Currently only placement new is supported.
			reader_.expect(Token::LROUNDBRACKET);
			auto placementArg = parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			auto value = parseValue(context);
			
			return builder_.makeNewValue(std::move(placementArg),
			                             std::move(value), start);
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseCallValue(const Context context) {
			const auto start = reader_.position();
			
			auto value = parseTypeValue(context);
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::DOT:
					case Token::PTRACCESS: {
						reader_.consume();
						const bool isDeref = (token.kind() == Token::PTRACCESS);
						value = parseMemberAccessExpression(std::move(value), isDeref,
						                                    start);
						break;
					}
					case Token::LROUNDBRACKET: {
						reader_.consume();
						auto valueList = parseValueList();
						reader_.expect(Token::RROUNDBRACKET);
						value = builder_.makeCallValue(std::move(value), std::move(valueList), start);
						break;
					}
					case Token::LSQUAREBRACKET: {
						reader_.consume();
						auto indexValue = parseValue();
						reader_.expect(Token::RSQUAREBRACKET);
						value = builder_.makeIndexValue(std::move(value), std::move(indexValue), start);
						break;
					}
					default:
						return value;
				}
			}
		}
		
		bool ValueParser::isCallValueOrNext(const AST::Node<AST::ValueDecl>& operand) const {
			switch (operand->kind()) {
				case AST::ValueDecl::SELF:
				case AST::ValueDecl::THIS:
				case AST::ValueDecl::SELFCONST:
				case AST::ValueDecl::BRACKET:
				case AST::ValueDecl::LITERAL:
				case AST::ValueDecl::SYMBOLREF:
				case AST::ValueDecl::TYPEREF:
				case AST::ValueDecl::MEMBERREF:
				case AST::ValueDecl::ALIGNOF:
				case AST::ValueDecl::SIZEOF:
				case AST::ValueDecl::CAST:
				case AST::ValueDecl::INTERNALCONSTRUCT:
				case AST::ValueDecl::ARRAYLITERAL:
					assert(isAtomicValue(operand));
					return true;
				case AST::ValueDecl::FUNCTIONCALL:
				case AST::ValueDecl::MEMBERACCESS:
				case AST::ValueDecl::TEMPLATEDMEMBERACCESS:
				case AST::ValueDecl::MERGE:
					assert(!isAtomicValue(operand));
					return true;
				case AST::ValueDecl::NEW:
				case AST::ValueDecl::UNARYOP:
				case AST::ValueDecl::BINARYOP:
				case AST::ValueDecl::TERNARY:
				case AST::ValueDecl::CAPABILITYTEST:
					assert(!isAtomicValue(operand));
					return operand->isIndex();
			}
			
			locic_unreachable("Invalid value kind");
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseMemberAccessExpression(AST::Node<AST::ValueDecl> value,
		                                                               const bool isDeref,
		                                                               const Debug::SourcePosition& start) {
			const auto memberName = FunctionParser(reader_).parseFunctionNameElement();
			auto templateArgs = SymbolParser(reader_).parseSymbolTemplateArgumentList();
			
			if (isDeref) {
				value = builder_.makeDerefValue(std::move(value), start);
			}
			
			if (templateArgs->empty()) {
				return builder_.makeMemberAccess(std::move(value), memberName, start);
			} else {
				return builder_.makeTemplatedMemberAccess(std::move(value), memberName,
				                                          std::move(templateArgs),
				                                          start);
			}
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseTypeValue(const Context context) {
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
						
						if (!canInterpretValueAsType(value)) {
							// Value cannot be interpreted as type,
							// so this must be multiply/bitwise-AND.
							return value;
						}
						
						auto type = interpretValueAsType(std::move(value));
						if (token.kind() == Token::STAR) {
							type = TypeBuilder(reader_).makePointerType(std::move(type), start);
						} else {
							type = TypeBuilder(reader_).makeReferenceType(std::move(type), start);
						}
						
						reader_.consume();
						
						value = builder_.makeTypeValue(std::move(type), start);
						break;
					}
					case Token::LSQUAREBRACKET: {
						if (!canInterpretValueAsType(value)) {
							return value;
						}
						
						reader_.consume();
						auto indexValue = parseValue();
						reader_.expect(Token::RSQUAREBRACKET);
						
						// typename_t doesn't support indexing, so this must be a type value.
						const bool isDefinitelyType = (value->kind() == AST::ValueDecl::TYPEREF);
						
						auto type = interpretValueAsType(isDefinitelyType ? std::move(value) : value.copy());
						auto staticArrayType =
						    TypeBuilder(reader_).makeStaticArrayType(std::move(type),
						                                             isDefinitelyType ? std::move(indexValue) : indexValue.copy(),
						                                             start);
						auto typeValue = builder_.makeTypeValue(std::move(staticArrayType), start);
						
						if (isDefinitelyType) {
							value = std::move(typeValue);
							break;
						}
						
						assert(value.get() != nullptr);
						assert(indexValue.get() != nullptr);
						
						// Ambiguous: could be indexing or a static array type.
						auto indexResult = builder_.makeIndexValue(std::move(value), std::move(indexValue), start);
						value = builder_.makeMergeValue(std::move(indexResult), std::move(typeValue), start);
						break;
					}
					default:
						return value;
				}
			}
		}
		
		bool ValueParser::canInterpretValueAsType(const AST::Node<AST::ValueDecl>& value) {
			switch (value->kind()) {
				case AST::ValueDecl::BRACKET:
					return canInterpretValueAsType(value->bracket.value);
				case AST::ValueDecl::SYMBOLREF:
				case AST::ValueDecl::TYPEREF:
					return true;
				case AST::ValueDecl::MERGE:
					return canInterpretValueAsType(value->merge.first) ||
					       canInterpretValueAsType(value->merge.second);
				case AST::ValueDecl::SELF:
				case AST::ValueDecl::THIS:
				case AST::ValueDecl::SELFCONST:
				case AST::ValueDecl::LITERAL:
				case AST::ValueDecl::MEMBERREF:
				case AST::ValueDecl::ALIGNOF:
				case AST::ValueDecl::SIZEOF:
				case AST::ValueDecl::NEW:
				case AST::ValueDecl::UNARYOP:
				case AST::ValueDecl::BINARYOP:
				case AST::ValueDecl::TERNARY:
				case AST::ValueDecl::CAST:
				case AST::ValueDecl::INTERNALCONSTRUCT:
				case AST::ValueDecl::MEMBERACCESS:
				case AST::ValueDecl::TEMPLATEDMEMBERACCESS:
				case AST::ValueDecl::FUNCTIONCALL:
				case AST::ValueDecl::CAPABILITYTEST:
				case AST::ValueDecl::ARRAYLITERAL:
					return false;
			}
			
			locic_unreachable("Invalid value kind");
		}
		
		AST::Node<AST::TypeDecl> ValueParser::interpretValueAsType(AST::Node<AST::ValueDecl> value) {
			switch (value->kind()) {
				case AST::ValueDecl::BRACKET:
					return interpretValueAsType(std::move(value->bracket.value));
				case AST::ValueDecl::SYMBOLREF:
					return AST::makeNode(value.location(),
					                     AST::TypeDecl::Object(std::move(value->symbolRef.symbol)));
				case AST::ValueDecl::TYPEREF:
					return std::move(value->typeRef.type);
				case AST::ValueDecl::MERGE:
					if (canInterpretValueAsType(value->merge.second)) {
						return interpretValueAsType(std::move(value->merge.second));
					} else {
						return interpretValueAsType(std::move(value->merge.first));
					}
				case AST::ValueDecl::SELF:
				case AST::ValueDecl::THIS:
				case AST::ValueDecl::SELFCONST:
				case AST::ValueDecl::LITERAL:
				case AST::ValueDecl::MEMBERREF:
				case AST::ValueDecl::ALIGNOF:
				case AST::ValueDecl::SIZEOF:
				case AST::ValueDecl::NEW:
				case AST::ValueDecl::UNARYOP:
				case AST::ValueDecl::BINARYOP:
				case AST::ValueDecl::TERNARY:
				case AST::ValueDecl::CAST:
				case AST::ValueDecl::INTERNALCONSTRUCT:
				case AST::ValueDecl::MEMBERACCESS:
				case AST::ValueDecl::TEMPLATEDMEMBERACCESS:
				case AST::ValueDecl::FUNCTIONCALL:
				case AST::ValueDecl::CAPABILITYTEST:
				case AST::ValueDecl::ARRAYLITERAL: {
					reader_.issueDiagWithLoc(CannotInterpretValueAsType(),
					                         value.location());
					return AST::makeNode(value.location(),
					                     AST::TypeDecl::Primitive(PrimitiveInt));
				}
			}
			
			locic_unreachable("Invalid value kind");
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseAtomicValue() {
			const auto start = reader_.position();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::LROUNDBRACKET: {
					if (reader_.peek(/*offset=*/1).kind() == Token::STAR &&
					    reader_.peek(/*offset=*/2).kind() == Token::RROUNDBRACKET) {
						// Function pointer type.
						break;
					}
					reader_.consume();
					auto value = parseValue();
					reader_.expect(Token::RROUNDBRACKET);
					return builder_.makeBracketedValue(std::move(value), start);
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
				case Token::SELF:
					reader_.consume();
					return builder_.makeSelfValue(start);
				case Token::THIS:
					reader_.consume();
					return builder_.makeThisValue(start);
				case Token::ALIGNOF: {
					reader_.consume();
					reader_.expect(Token::LROUNDBRACKET);
					auto type = TypeParser(reader_).parseType();
					reader_.expect(Token::RROUNDBRACKET);
					return builder_.makeAlignOfValue(std::move(type), start);
				}
				case Token::SIZEOF: {
					reader_.consume();
					reader_.expect(Token::LROUNDBRACKET);
					auto type = TypeParser(reader_).parseType();
					reader_.expect(Token::RROUNDBRACKET);
					return builder_.makeSizeOfValue(std::move(type), start);
				}
				case Token::LCURLYBRACKET:
					reader_.consume();
					return parseArrayLiteral(start);
				case Token::CONST_CAST:
				case Token::DYNAMIC_CAST:
				case Token::REINTERPRET_CAST: {
					return parseCastValue();
				}
				case Token::SELFCONST: {
					const auto next = reader_.peek(/*offset=*/1).kind();
					if (TypeParser(reader_).isTypeStartToken(next)) {
						break;
					}
					reader_.consume();
					return builder_.makeSelfConstValue(start);
				}
				default:
					break;
			}
			
			if (TypeParser(reader_).isTypeStartToken(token.kind())) {
				auto type = TypeParser(reader_).parseType();
				return builder_.makeTypeValue(std::move(type), start);
			}
			
			reader_.issueDiag(InvalidValidDiag(token.kind()), start);
			
			if (token.kind() != Token::END) {
				reader_.consume();
			}
			
			// Pretend we got a 'null' value.
			return builder_.makeLiteralValue(Constant::Null(),
			                                 reader_.makeCString(""),
			                                 start);
		}
		
		bool ValueParser::isAtomicValue(const AST::Node<AST::ValueDecl>& operand) const {
			switch (operand->kind()) {
				case AST::ValueDecl::SELF:
				case AST::ValueDecl::THIS:
				case AST::ValueDecl::SELFCONST:
				case AST::ValueDecl::BRACKET:
				case AST::ValueDecl::LITERAL:
				case AST::ValueDecl::SYMBOLREF:
				case AST::ValueDecl::TYPEREF:
				case AST::ValueDecl::MEMBERREF:
				case AST::ValueDecl::ALIGNOF:
				case AST::ValueDecl::SIZEOF:
				case AST::ValueDecl::CAST:
				case AST::ValueDecl::INTERNALCONSTRUCT:
				case AST::ValueDecl::ARRAYLITERAL:
					return true;
				case AST::ValueDecl::NEW:
				case AST::ValueDecl::UNARYOP:
				case AST::ValueDecl::BINARYOP:
				case AST::ValueDecl::TERNARY:
				case AST::ValueDecl::MEMBERACCESS:
				case AST::ValueDecl::TEMPLATEDMEMBERACCESS:
				case AST::ValueDecl::FUNCTIONCALL:
				case AST::ValueDecl::CAPABILITYTEST:
				case AST::ValueDecl::MERGE:
					return false;
			}
			
			locic_unreachable("Invalid value kind");
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseAtExpression(const Debug::SourcePosition& start) {
			const auto token = reader_.peek();
			if (token.kind() == Token::NAME) {
				reader_.consume();
				return builder_.makeSelfMemberAccess(token.name(), start);
			}
			
			auto templateArguments = parseOptionalTemplateArguments();
			
			reader_.expect(Token::LROUNDBRACKET);
			auto arguments = parseValueList();
			reader_.expect(Token::RROUNDBRACKET);
			return builder_.makeInternalConstruct(std::move(templateArguments),
			                                      std::move(arguments), start);
		}
		
		AST::Node<AST::ValueDeclList> ValueParser::parseOptionalTemplateArguments() {
			const auto start = reader_.position();
			
			if (reader_.peek().kind() != Token::LTRIBRACKET) {
				return builder_.makeValueList(AST::ValueDeclList(), start);
			}
			
			reader_.consume();
			auto valueList = parseValueList(IN_TEMPLATE);
			reader_.expect(Token::RTRIBRACKET);
			
			return valueList;
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseSymbolOrLiteralValue() {
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
					auto symbol = SymbolParser(reader_).parseSymbol();
					return builder_.makeSymbolValue(std::move(symbol), start);
				}
			}
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseLiteral(const Constant constant,
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
		
		AST::Node<AST::ValueDecl> ValueParser::parseArrayLiteral(const Debug::SourcePosition& start) {
			auto valueList = parseValueList();
			reader_.expect(Token::RCURLYBRACKET);
			return builder_.makeArrayLiteralValue(std::move(valueList), start);
		}
		
		AST::Node<AST::ValueDeclList> ValueParser::parseValueList(const Context context) {
			const auto start = reader_.position();
			
			AST::ValueDeclList valueList;
			valueList.reserve(8);
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::RCURLYBRACKET:
					case Token::RTRIBRACKET:
					case Token::RROUNDBRACKET:
						return builder_.makeValueList(std::move(valueList), start);
					default:
						break;
				}
				
				valueList.push_back(parseValue(context));
				
				if (reader_.peek().kind() != Token::COMMA) {
					return builder_.makeValueList(std::move(valueList), start);
				}
				
				reader_.consume();
			}
		}
		
		AST::Node<AST::ValueDecl> ValueParser::parseCastValue() {
			const auto start = reader_.position();
			
			auto tokens = {
				Token::CONST_CAST,
				Token::DYNAMIC_CAST,
				Token::REINTERPRET_CAST
			};
			
			auto kind = AST::ValueDecl::CAST_CONST;
			
			const auto token = reader_.expectOneOf(tokens);
			switch (token.kind()) {
				case Token::CONST_CAST:
					kind = AST::ValueDecl::CAST_CONST;
					break;
				case Token::DYNAMIC_CAST:
					kind = AST::ValueDecl::CAST_DYNAMIC;
					break;
				case Token::REINTERPRET_CAST:
					kind = AST::ValueDecl::CAST_REINTERPRET;
					break;
				default:
					break;
			}
			
			reader_.expect(Token::LTRIBRACKET);
			auto fromType = TypeParser(reader_).parseType();
			reader_.expect(Token::COMMA);
			auto toType = TypeParser(reader_).parseType();
			reader_.expect(Token::RTRIBRACKET);
			
			reader_.expect(Token::LROUNDBRACKET);
			auto value = parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makeCastValue(kind, std::move(fromType),
			                              std::move(toType), std::move(value), start);
		}
		
	}
	
}
