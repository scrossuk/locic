#ifndef LOCIC_LEX_TOKEN_HPP
#define LOCIC_LEX_TOKEN_HPP

#include <cassert>

#include <locic/Constant.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

namespace locic {
	
	namespace Lex {
		
		class Token {
		public:
			enum Kind {
				END,
				
				NAME,
				VERSION,
				CONSTANT,
				
				TRUEVAL,
				FALSEVAL,

				UNKNOWN,
				ERROR,
				INTERFACE,
				SEMICOLON,
				NAMESPACE,
				UNDERSCORE,

				LCURLYBRACKET,
				RCURLYBRACKET,
				LSQUAREBRACKET,
				RSQUAREBRACKET,
				LROUNDBRACKET,
				RROUNDBRACKET,
				DOUBLE_LTRIBRACKET,
				LTRIBRACKET,
				RTRIBRACKET,

				AUTO,
				STATIC,
				IMPORT,
				EXPORT,
				MOVE,
				NEW,

				LVAL,
				NOLVAL,
				REF,
				STATICREF,
				NOREF,
				NOTAG,

				TEMPLATE,
				TYPENAME,
				VIRTUAL,
				REQUIRE,
				UNUSED,
				UNUSED_RESULT,

				USING,
				LET,
				ENUM,
				UNION,
				CASE,
				SWITCH,
				DEFAULT,
				CONTINUE,
				BREAK,
				EXCEPTION,
				THROW,
				TRY,
				CATCH,
				SCOPE,
				NOEXCEPT,

				ALIGNOF,
				SIZEOF,
				TYPEOF,
				TYPEID,

				PRIMITIVE,
				PRIMITIVEFUNCTION,

				STRUCT,
				CLASS,
				DATATYPE,

				SIGNED,
				UNSIGNED,
				BYTE,
				UBYTE,
				SHORT,
				USHORT,
				INT,
				UINT,
				LONG,
				ULONG,
				LONGLONG,
				ULONGLONG,
				FLOAT,
				DOUBLE,
				UNICHAR,

				COLON,
				DOUBLE_COLON,
				VOID,
				BOOL,
				CONST,
				MUTABLE,
				OVERRIDE_CONST,

				STAR,
				COMMA,
				IF,
				ELSE,
				FOR,
				WHILE,

				SETEQUAL,
				ADDEQUAL,
				SUBEQUAL,
				MULEQUAL,
				DIVEQUAL,
				PERCENTEQUAL,

				RETURN,
				ASSERT,
				UNREACHABLE,
				AT,
				NULLVAL,

				CONST_CAST,
				DYNAMIC_CAST,
				REINTERPRET_CAST,

				AND,
				OR,
				XOR,

				IS_A,
				DOT,
				PTRACCESS,
				PLUS,
				DOUBLE_PLUS,
				MINUS,
				DOUBLE_MINUS,
				EXCLAIMMARK,
				AMPERSAND,
				DOUBLE_AMPERSAND,
				VERTICAL_BAR,
				DOUBLE_VERTICAL_BAR,
				FORWARDSLASH,
				PERCENT,
				ISEQUAL,
				NOTEQUAL,
				GREATEROREQUAL,
				LESSOREQUAL,
				QUESTIONMARK,
				TILDA,
				CARET,

				SELF,
				THIS,

				INHERIT,
				OVERRIDE
			};
			
			static Token Name(const String stringValue,
			                  const Debug::SourceRange sourceRange = Debug::SourceRange::Null()) {
				Token token(NAME, sourceRange);
				token.data_.stringValue = stringValue;
				return token;
			}
			
			static Token Version(const Version versionValue,
			                     const Debug::SourceRange sourceRange = Debug::SourceRange::Null()) {
				Token token(VERSION, sourceRange);
				token.data_.versionValue = versionValue;
				return token;
			}
			
			static Token Constant(Constant constantValue,
			                      const Debug::SourceRange sourceRange = Debug::SourceRange::Null()) {
				Token token(CONSTANT, sourceRange);
				token.constantValue_ = std::move(constantValue);
				return token;
			}
			
			static Token Basic(const Kind kind,
			                   const Debug::SourceRange sourceRange = Debug::SourceRange::Null()) {
				Token token(kind, sourceRange);
				assert(!token.hasAssociatedData());
				return token;
			}
			
			Kind kind() const {
				return kind_;
			}
			
			bool isEnd() const {
				return kind() == END;
			}
			
			bool hasAssociatedData() const {
				switch (kind()) {
					case NAME:
					case VERSION:
					case CONSTANT:
						return true;
					default:
						return false;
				}
			}
			
			const String& name() const {
				assert(kind() == NAME);
				return data_.stringValue;
			}
			
			const class Version& version() const {
				assert(kind() == VERSION);
				return data_.versionValue;
			}
			
			const class Constant& constant() const {
				assert(kind() == CONSTANT);
				return constantValue_;
			}
			
			void setSourceRange(const Debug::SourceRange argSourceRange) {
				sourceRange_ = argSourceRange;
			}
			
			const Debug::SourceRange& sourceRange() const {
				return sourceRange_;
			}
			
			bool operator==(const Token& other) const {
				if (kind() != other.kind()) {
					return false;
				}
				
				if (sourceRange() != other.sourceRange()) {
					return false;
				}
				
				switch (kind()) {
					case NAME:
						return name() == other.name();
					case VERSION:
						return version() == other.version();
					case CONSTANT:
						return constant() == other.constant();
					default:
						assert(!hasAssociatedData());
						return true;
				}
			}
			
			static std::string kindToString(const Kind kind) {
				switch (kind) {
					case END: return "END";
					case NAME: return "NAME";
					case VERSION: return "VERSION";
					case CONSTANT: return "CONSTANT";
					case TRUEVAL: return "TRUEVAL";
					case FALSEVAL: return "FALSEVAL";
					case UNKNOWN: return "UNKNOWN";
					case ERROR: return "ERROR";
					case INTERFACE: return "INTERFACE";
					case SEMICOLON: return "SEMICOLON";
					case NAMESPACE: return "NAMESPACE";
					case UNDERSCORE: return "UNDERSCORE";
					case LCURLYBRACKET: return "LCURLYBRACKET";
					case RCURLYBRACKET: return "RCURLYBRACKET";
					case LSQUAREBRACKET: return "LSQUAREBRACKET";
					case RSQUAREBRACKET: return "RSQUAREBRACKET";
					case LROUNDBRACKET: return "LROUNDBRACKET";
					case RROUNDBRACKET: return "RROUNDBRACKET";
					case DOUBLE_LTRIBRACKET: return "DOUBLE_LTRIBRACKET";
					case LTRIBRACKET: return "LTRIBRACKET";
					case RTRIBRACKET: return "RTRIBRACKET";
					case AUTO: return "AUTO";
					case STATIC: return "STATIC";
					case IMPORT: return "IMPORT";
					case EXPORT: return "EXPORT";
					case MOVE: return "MOVE";
					case LVAL: return "LVAL";
					case NOLVAL: return "NOLVAL";
					case NEW: return "NEW";
					case REF: return "REF";
					case STATICREF: return "STATICREF";
					case NOREF: return "NOREF";
					case NOTAG: return "NOTAG";
					case TEMPLATE: return "TEMPLATE";
					case TYPENAME: return "TYPENAME";
					case VIRTUAL: return "VIRTUAL";
					case REQUIRE: return "REQUIRE";
					case UNUSED: return "UNUSED";
					case UNUSED_RESULT: return "UNUSED_RESULT";
					case USING: return "USING";
					case LET: return "LET";
					case ENUM: return "ENUM";
					case UNION: return "UNION";
					case CASE: return "CASE";
					case SWITCH: return "SWITCH";
					case DEFAULT: return "DEFAULT";
					case CONTINUE: return "CONTINUE";
					case BREAK: return "BREAK";
					case EXCEPTION: return "EXCEPTION";
					case THROW: return "THROW";
					case TRY: return "TRY";
					case CATCH: return "CATCH";
					case SCOPE: return "SCOPE";
					case NOEXCEPT: return "NOEXCEPT";
					case ALIGNOF: return "ALIGNOF";
					case SIZEOF: return "SIZEOF";
					case TYPEOF: return "TYPEOF";
					case TYPEID: return "TYPEID";
					case PRIMITIVE: return "PRIMITIVE";
					case PRIMITIVEFUNCTION: return "PRIMITIVEFUNCTION";
					case STRUCT: return "STRUCT";
					case CLASS: return "CLASS";
					case DATATYPE: return "DATATYPE";
					case SIGNED: return "SIGNED";
					case UNSIGNED: return "UNSIGNED";
					case BYTE: return "BYTE";
					case UBYTE: return "UBYTE";
					case SHORT: return "SHORT";
					case USHORT: return "USHORT";
					case INT: return "INT";
					case UINT: return "UINT";
					case LONG: return "LONG";
					case ULONG: return "ULONG";
					case LONGLONG: return "LONGLONG";
					case ULONGLONG: return "ULONGLONG";
					case FLOAT: return "FLOAT";
					case DOUBLE: return "DOUBLE";
					case UNICHAR: return "UNICHAR";
					case COLON: return "COLON";
					case DOUBLE_COLON: return "DOUBLE_COLON";
					case VOID: return "VOID";
					case BOOL: return "BOOL";
					case CONST: return "CONST";
					case MUTABLE: return "MUTABLE";
					case OVERRIDE_CONST: return "OVERRIDE_CONST";
					case STAR: return "STAR";
					case COMMA: return "COMMA";
					case IF: return "IF";
					case ELSE: return "ELSE";
					case FOR: return "FOR";
					case WHILE: return "WHILE";
					case SETEQUAL: return "SETEQUAL";
					case ADDEQUAL: return "ADDEQUAL";
					case SUBEQUAL: return "SUBEQUAL";
					case MULEQUAL: return "MULEQUAL";
					case DIVEQUAL: return "DIVEQUAL";
					case PERCENTEQUAL: return "PERCENTEQUAL";
					case RETURN: return "RETURN";
					case ASSERT: return "ASSERT";
					case UNREACHABLE: return "UNREACHABLE";
					case AT: return "AT";
					case NULLVAL: return "NULLVAL";
					case CONST_CAST: return "CONST_CAST";
					case DYNAMIC_CAST: return "DYNAMIC_CAST";
					case REINTERPRET_CAST: return "REINTERPRET_CAST";
					case AND: return "AND";
					case OR: return "OR";
					case XOR: return "XOR";
					case IS_A: return "IS_A";
					case DOT: return "DOT";
					case PTRACCESS: return "PTRACCESS";
					case PLUS: return "PLUS";
					case DOUBLE_PLUS: return "DOUBLE_PLUS";
					case MINUS: return "MINUS";
					case DOUBLE_MINUS: return "DOUBLE_MINUS";
					case EXCLAIMMARK: return "EXCLAIMMARK";
					case AMPERSAND: return "AMPERSAND";
					case DOUBLE_AMPERSAND: return "DOUBLE_AMPERSAND";
					case VERTICAL_BAR: return "VERTICAL_BAR";
					case DOUBLE_VERTICAL_BAR: return "DOUBLE_VERTICAL_BAR";
					case FORWARDSLASH: return "FORWARDSLASH";
					case PERCENT: return "PERCENT";
					case ISEQUAL: return "ISEQUAL";
					case NOTEQUAL: return "NOTEQUAL";
					case GREATEROREQUAL: return "GREATEROREQUAL";
					case LESSOREQUAL: return "LESSOREQUAL";
					case QUESTIONMARK: return "QUESTIONMARK";
					case TILDA: return "TILDA";
					case CARET: return "CARET";
					case SELF: return "SELF";
					case THIS: return "THIS";
					case INHERIT: return "INHERIT";
					case OVERRIDE: return "OVERRIDE";
				}
				
				locic_unreachable("Invalid token kind.");
			}
			
			std::string toString() const {
				return kindToString(kind());
			}
			
		private:
			Token(const Kind argKind, const Debug::SourceRange argSourceRange)
			: kind_(argKind), sourceRange_(argSourceRange) { }
			
			Kind kind_;
			Debug::SourceRange sourceRange_;
			class Constant constantValue_;
			
			union {
				String stringValue;
				class Version versionValue;
			} data_;
			
		};
		
	}
	
}

#endif