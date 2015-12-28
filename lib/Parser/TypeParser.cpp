#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeParser.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		TypeParser::TypeParser(TokenReader& reader)
		: reader_(reader) { }
		
		TypeParser::~TypeParser() { }
		
		AST::Node<AST::Type> TypeParser::parseType() {
			const auto start = reader_.position();
			
			auto type = parseQualifiedType();
			
			while (true) {
				const auto token = reader_.peek();
				switch (token.kind()) {
					case Token::STAR:
						reader_.consume();
						type = makePointerType(type, start);
						break;
					case Token::AMPERSAND:
						reader_.consume();
						type = makeReferenceType(type, start);
						break;
					case Token::LSQUAREBRACKET:
						reader_.consume();
						throw std::logic_error("TODO");
					default:
						return type;
				}
			}
		}
		
		AST::Node<AST::Type> TypeParser::parseQualifiedType() {
			const auto start = reader_.position();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::CONST:
					reader_.consume();
					return parseConstType();
				case Token::LVAL:
				case Token::REF:
				case Token::STATICREF:
					reader_.consume();
					return parseTypeWithQualifier(start, token.kind());
				case Token::LROUNDBRACKET: {
					reader_.consume();
					const auto type = parseType();
					reader_.expect(Token::RROUNDBRACKET);
					return type;
				}
				default:
					return parseBasicType();
			}
		}
		
		AST::Node<AST::Type> TypeParser::parseConstType() {
			throw std::logic_error("TODO");
		}
		
		AST::Node<AST::Type> TypeParser::parseTypeWithQualifier(const Debug::SourcePosition& start,
		                                                        const Token::Kind qualifier) {
			reader_.expect(Token::LTRIBRACKET);
			
			const auto targetType = parseType();
			
			reader_.expect(Token::RTRIBRACKET);
			
			const auto type = parseQualifiedType();
			
			switch (qualifier) {
				case Token::LVAL:
					return makeLvalType(targetType, type, start);
				case Token::REF:
					return makeRefType(targetType, type, start);
				case Token::STATICREF:
					return makeStaticRefType(targetType, type, start);
				default:
					throw std::logic_error("Unknown type qualifier kind.");
			}
		}
		
		AST::Node<AST::Type> TypeParser::parseBasicType() {
			const auto start = reader_.position();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::AUTO:
					reader_.consume();
					return makeAutoType(start);
				case Token::VOID:
					reader_.consume();
					return makePrimitiveType(PrimitiveVoid, start);
				case Token::BOOL:
					reader_.consume();
					return makePrimitiveType(PrimitiveBool, start);
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
					return makePrimitiveType(PrimitiveUByte, start);
				case Token::USHORT:
					reader_.consume();
					return makePrimitiveType(PrimitiveUShort, start);
				case Token::UINT:
					reader_.consume();
					return makePrimitiveType(PrimitiveUInt, start);
				case Token::ULONG:
					reader_.consume();
					return makePrimitiveType(PrimitiveULong, start);
				case Token::ULONGLONG:
					reader_.consume();
					return makePrimitiveType(PrimitiveULongLong, start);
				case Token::FLOAT:
					reader_.consume();
					return makePrimitiveType(PrimitiveFloat, start);
				case Token::DOUBLE:
					reader_.consume();
					return makePrimitiveType(PrimitiveDouble, start);
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
				case Token::NAME:
					reader_.consume();
					return makeNamedType(token.name(), start);
				default:
					issueError(Diag::InvalidType, start);
					reader_.consume();
					
					// Pretend we got an int type.
					return makePrimitiveType(PrimitiveInt, start);
			}
		}
		
		AST::Node<AST::Type> TypeParser::parseLongIntOrFloatType(const Debug::SourcePosition& start) {
			switch (reader_.peek().kind()) {
				case Token::DOUBLE:
					reader_.consume();
					return makePrimitiveType(PrimitiveLongDouble, start);
				default:
					return parseLongIntegerType(start,
					                            /*isSigned=*/true);
			}
		}
		
		AST::Node<AST::Type> TypeParser::parseIntegerTypeWithSignedness(const Debug::SourcePosition& start,
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
							issueError(Diag::DuplicateSignedQualifier, start);
						} else {
							issueError(Diag::ConflictingSignedQualifier, start);
						}
						break;
					}
					case Token::BYTE:
						reader_.consume();
						return makePrimitiveType(PrimitiveByte, start, isSigned);
					case Token::SHORT:
						reader_.consume();
						if (reader_.peek().kind() == Token::INT) {
							reader_.consume();
						}
						return makePrimitiveType(PrimitiveShort, start, isSigned);
					case Token::INT:
						reader_.consume();
						return makePrimitiveType(PrimitiveInt, start, isSigned);
					case Token::LONG:
						reader_.consume();
						return parseLongIntegerType(start, isSigned);
					case Token::LONGLONG:
						reader_.consume();
						return makePrimitiveType(PrimitiveLongLong, start, isSigned);
					default:
						return makePrimitiveType(PrimitiveInt, start, isSigned);
				}
			}
		}
		
		AST::Node<AST::Type> TypeParser::parseLongIntegerType(const Debug::SourcePosition& start,
		                                                      const bool isSigned) {
			auto primitiveKind = PrimitiveLong;
			
			if (reader_.peek().kind() == Token::LONG) {
				primitiveKind = PrimitiveLongLong;
				reader_.consume();
			}
			
			if (reader_.peek().kind() == Token::INT) {
				reader_.consume();
			}
			
			return makePrimitiveType(primitiveKind, start, isSigned);
		}
		
	}
	
}