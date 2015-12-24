#ifndef LOCIC_LEX_TOKEN_HPP
#define LOCIC_LEX_TOKEN_HPP

#include <cassert>

#include <locic/Constant.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

namespace locic {
	
	namespace Lex {
		
		class Token {
		public:
			enum Kind {
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

				COLON,
				DOUBLE_COLON,
				VOID,
				BOOL,
				FINAL,
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

				SELF,
				THIS
			};
			
			static Token Name(const String stringValue) {
				Token token(NAME);
				token.data_.stringValue = stringValue;
				return token;
			}
			
			static Token Version(const Version versionValue) {
				Token token(VERSION);
				token.data_.versionValue = versionValue;
				return token;
			}
			
			static Token Constant(const Constant constantValue) {
				Token token(CONSTANT);
				token.data_.constantValue = constantValue;
				return token;
			}
			
			static Token Basic(const Kind kind) {
				Token token(kind);
				assert(!token.hasAssociatedData());
				return token;
			}
			
			Kind kind() const {
				return kind_;
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
				return data_.constantValue;
			}
			
		private:
			Token(const Kind argKind)
			: kind_(argKind) { }
			
			Kind kind_;
			
			union {
				String stringValue;
				class Constant constantValue;
				class Version versionValue;
			} data_;
			
		};
		
	}
	
}

#endif