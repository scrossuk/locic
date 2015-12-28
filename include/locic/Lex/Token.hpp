#ifndef LOCIC_LEX_TOKEN_HPP
#define LOCIC_LEX_TOKEN_HPP

#include <cassert>

#include <locic/Constant.hpp>
#include <locic/Debug/SourceRange.hpp>
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

				SELF,
				THIS
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
			
			static Token Constant(const Constant constantValue,
			                      const Debug::SourceRange sourceRange = Debug::SourceRange::Null()) {
				Token token(CONSTANT, sourceRange);
				token.data_.constantValue = constantValue;
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
				return data_.constantValue;
			}
			
			void setSourceRange(const Debug::SourceRange sourceRange) {
				sourceRange_ = sourceRange;
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
			
		private:
			Token(const Kind argKind, const Debug::SourceRange argSourceRange)
			: kind_(argKind), sourceRange_(argSourceRange) { }
			
			Kind kind_;
			Debug::SourceRange sourceRange_;
			
			union {
				String stringValue;
				class Constant constantValue;
				class Version versionValue;
			} data_;
			
		};
		
	}
	
}

#endif