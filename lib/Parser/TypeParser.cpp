#include <locic/AST.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Parser/PredicateParser.hpp>
#include <locic/Parser/SymbolParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeBuilder.hpp>
#include <locic/Parser/TypeParser.hpp>
#include <locic/Parser/ValueParser.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		class InvalidTypeDiag: public Error {
		public:
			InvalidTypeDiag(const Token::Kind actual)
			: actual_(actual) { }
			
			std::string toString() const {
				return makeString("unexpected type token: %s",
				                  Token::kindToString(actual_).c_str());
			}
			
		private:
			Token::Kind actual_;
			
		};
		
		class DuplicateSignedQualifierDiag: public Warning {
		public:
			DuplicateSignedQualifierDiag() { }
			
			std::string toString() const {
				return "duplicate 'signed' qualifier";
			}
			
		};
		
		class ConflictingSignedQualifierDiag: public Warning {
		public:
			ConflictingSignedQualifierDiag() { }
			
			std::string toString() const {
				return "conflicting 'signed' and 'unsigned' qualifiers";
			}
			
		};
		
		TypeParser::TypeParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		TypeParser::~TypeParser() { }
		
		AST::Node<AST::TypeDecl> TypeParser::parseType() {
			const auto start = reader_.position();
			auto type = parseQualifiedType();
			return parseIndirectTypeBasedOnType(std::move(type), start);
		}
		
		AST::Node<AST::TypeDecl> TypeParser::parseIndirectTypeBasedOnType(AST::Node<AST::TypeDecl> type,
		                                                              const Debug::SourcePosition& start) {
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::STAR:
						reader_.consume();
						type = builder_.makePointerType(std::move(type), start);
						break;
					case Token::AMPERSAND:
						reader_.consume();
						type = builder_.makeReferenceType(std::move(type), start);
						break;
					case Token::LSQUAREBRACKET: {
						reader_.consume();
						auto sizeValue = ValueParser(reader_).parseValue();
						reader_.expect(Token::RSQUAREBRACKET);
						type = builder_.makeStaticArrayType(std::move(type), std::move(sizeValue), start);
						break;
					}
					default:
						return type;
				}
			}
		}
		
		AST::Node<AST::TypeDecl> TypeParser::parseQualifiedType() {
			const auto start = reader_.position();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::CONST:
					return parseConstType();
				case Token::NOTAG: {
					reader_.consume();
					auto targetType = parseQualifiedType();
					return builder_.makeNoTagType(std::move(targetType), start);
				}
				case Token::LVAL:
				case Token::REF:
				case Token::STATICREF:
					reader_.consume();
					return parseTypeWithQualifier(start, token.kind());
				case Token::LROUNDBRACKET: {
					if (reader_.peek(/*offset=*/1).kind() == Token::STAR) {
						return parseFunctionPointerType();
					}
					reader_.consume();
					auto type = parseType();
					reader_.expect(Token::RROUNDBRACKET);
					return type;
				}
				default:
					return parseBasicType();
			}
		}
		
		AST::Node<AST::TypeDecl> TypeParser::parseConstType() {
			const auto start = reader_.position();
			
			reader_.expect(Token::CONST);
			
			if (reader_.peek().kind() == Token::LTRIBRACKET) {
				reader_.consume();
				auto predicate = PredicateParser(reader_).parsePredicate();
				reader_.expect(Token::RTRIBRACKET);
				auto targetType = parseQualifiedType();
				return builder_.makeConstPredicateType(std::move(predicate), std::move(targetType),
				                                       start);
			}
			
			auto targetType = parseQualifiedType();
			return builder_.makeConstType(std::move(targetType), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeParser::parseTypeWithQualifier(const Debug::SourcePosition& start,
		                                   const Token::Kind qualifier) {
			reader_.expect(Token::LTRIBRACKET);
			
			auto targetType = parseType();
			
			reader_.expect(Token::RTRIBRACKET);
			
			auto type = parseQualifiedType();
			
			switch (qualifier) {
				case Token::LVAL:
					return builder_.makeLvalType(std::move(targetType), std::move(type), start);
				case Token::REF:
					return builder_.makeRefType(std::move(targetType), std::move(type), start);
				case Token::STATICREF:
					return builder_.makeStaticRefType(std::move(targetType), std::move(type), start);
				default:
					locic_unreachable("Unknown type qualifier kind.");
			}
		}
		
		AST::Node<AST::TypeDecl> TypeParser::parseFunctionPointerType() {
			const auto start = reader_.position();
			
			reader_.expect(Token::LROUNDBRACKET);
			reader_.expect(Token::STAR);
			reader_.expect(Token::RROUNDBRACKET);
			
			reader_.expect(Token::LROUNDBRACKET);
			auto returnType = parseType();
			reader_.expect(Token::RROUNDBRACKET);
			
			reader_.expect(Token::LROUNDBRACKET);
			auto paramTypes = parseTypeList();
			const bool isVarArg = (reader_.peek().kind() == Token::COMMA);
			if (isVarArg) {
				reader_.expect(Token::COMMA);
				reader_.expect(Token::DOT);
				reader_.expect(Token::DOT);
				reader_.expect(Token::DOT);
			}
			reader_.expect(Token::RROUNDBRACKET);
			
			return builder_.makeFunctionPointerType(std::move(returnType), std::move(paramTypes),
			                                        isVarArg, start);
		}
		
		AST::Node<AST::TypeDeclList> TypeParser::parseTypeList() {
			const auto start = reader_.position();
			
			AST::TypeDeclList list;
			list.reserve(8);
			
			if (reader_.peek().kind() != Token::RROUNDBRACKET) {
				list.push_back(parseType());
				
				while (reader_.peek().kind() != Token::RROUNDBRACKET) {
					if (reader_.peek().kind() == Token::COMMA &&
					    reader_.peek(/*offset=*/1).kind() == Token::DOT) {
						// This is a var args list.
						break;
					}
					
					reader_.expect(Token::COMMA);
					list.push_back(parseType());
				}
			}
			
			return builder_.makeTypeList(std::move(list), start);
		}
		
		AST::Node<AST::TypeDecl> TypeParser::parseBasicType() {
			const auto start = reader_.position();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::AUTO:
					reader_.consume();
					return builder_.makeAutoType(start);
				case Token::VOID:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveVoid, start);
				case Token::BOOL:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveBool, start);
				case Token::TYPENAME:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveTypename, start);
				case Token::UNSIGNED:
					reader_.consume();
					return parseIntegerTypeWithSignedness(start,
					                                      /*isSigned=*/false);
				case Token::SIGNED:
					reader_.consume();
					return parseIntegerTypeWithSignedness(start,
					                                      /*isSigned=*/true);
				case Token::UBYTE:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveUByte, start);
				case Token::USHORT:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveUShort, start);
				case Token::UINT:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveUInt, start);
				case Token::ULONG:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveULong, start);
				case Token::ULONGLONG:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveULongLong, start);
				case Token::FLOAT:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveFloat, start);
				case Token::DOUBLE:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveDouble, start);
				case Token::LONG:
					reader_.consume();
					return parseLongIntOrFloatType(start);
				case Token::BYTE:
				case Token::SHORT:
				case Token::INT:
				case Token::LONGLONG:
					// Default to 'signed'.
					return parseIntegerTypeWithSignedness(start,
					                                      /*isSigned=*/true);
				case Token::UNICHAR:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveUnichar, start);
				case Token::NAME: {
					auto symbol = SymbolParser(reader_).parseSymbol(SymbolParser::IN_TYPE);
					return builder_.makeSymbolType(std::move(symbol), start);
				}
				default:
					reader_.issueDiag(InvalidTypeDiag(token.kind()), start);
					if (token.kind() != Token::END) {
						reader_.consume();
					}
					
					// Pretend we got an int type.
					return builder_.makePrimitiveType(PrimitiveInt, start);
			}
		}
		
		AST::Node<AST::TypeDecl> TypeParser::parseLongIntOrFloatType(const Debug::SourcePosition& start) {
			switch (reader_.peek().kind()) {
				case Token::DOUBLE:
					reader_.consume();
					return builder_.makePrimitiveType(PrimitiveLongDouble, start);
				default:
					return parseLongIntegerType(start,
					                            /*isSigned=*/true);
			}
		}
		
		AST::Node<AST::TypeDecl> TypeParser::parseIntegerTypeWithSignedness(const Debug::SourcePosition& start,
		                                                                const bool isSigned) {
			// A loop allows us to scan in any repeated 'signed' qualifiers.
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::UNSIGNED:
					case Token::SIGNED: {
						reader_.consume();
						const bool currentIsSigned = (token.kind() == Token::SIGNED);
						if (currentIsSigned == isSigned) {
							reader_.issueDiag(DuplicateSignedQualifierDiag(), start);
						} else {
							reader_.issueDiag(ConflictingSignedQualifierDiag(), start);
						}
						break;
					}
					case Token::BYTE: {
						reader_.consume();
						const auto primitiveID = isSigned ? PrimitiveByte : PrimitiveUByte;
						return builder_.makePrimitiveType(primitiveID, start);
					}
					case Token::SHORT: {
						reader_.consume();
						if (reader_.peek().kind() == Token::INT) {
							reader_.consume();
						}
						const auto primitiveID = isSigned ? PrimitiveShort : PrimitiveUShort;
						return builder_.makePrimitiveType(primitiveID, start);
					}
					case Token::INT: {
						reader_.consume();
						const auto primitiveID = isSigned ? PrimitiveInt : PrimitiveUInt;
						return builder_.makePrimitiveType(primitiveID, start);
					}
					case Token::LONG:
						reader_.consume();
						return parseLongIntegerType(start, isSigned);
					case Token::LONGLONG: {
						reader_.consume();
						const auto primitiveID = isSigned ? PrimitiveLongLong : PrimitiveULongLong;
						return builder_.makePrimitiveType(primitiveID, start);
					}
					default: {
						const auto primitiveID = isSigned ? PrimitiveInt : PrimitiveUInt;
						return builder_.makePrimitiveType(primitiveID, start);
					}
				}
			}
		}
		
		AST::Node<AST::TypeDecl> TypeParser::parseLongIntegerType(const Debug::SourcePosition& start,
		                                                      const bool isSigned) {
			auto primitiveKind = isSigned ? PrimitiveLong : PrimitiveULong;
			
			if (reader_.peek().kind() == Token::LONG) {
				primitiveKind = isSigned ? PrimitiveLongLong : PrimitiveULongLong;
				reader_.consume();
			}
			
			if (reader_.peek().kind() == Token::INT) {
				reader_.consume();
			}
			
			return builder_.makePrimitiveType(primitiveKind, start);
		}
		
		bool TypeParser::isTypeStartToken(const Token::Kind kind) {
			switch (kind) {
				case Token::AUTO:
				case Token::VOID:
				case Token::BOOL:
				case Token::TYPENAME:
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
				case Token::SIGNED:
				case Token::UNSIGNED:
				case Token::FLOAT:
				case Token::DOUBLE:
				case Token::UNICHAR:
				case Token::CONST:
				case Token::LVAL:
				case Token::REF:
				case Token::STATICREF:
				case Token::NAME:
				case Token::NOTAG:
				case Token::LROUNDBRACKET:
					return true;
				default:
					return false;
			}
		}
		
	}
	
}
