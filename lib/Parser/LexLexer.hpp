#ifndef LOCIC_PARSER_LEXLEXER_HPP
#define LOCIC_PARSER_LEXLEXER_HPP

#include <cstdio>

#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Lex/DiagnosticReceiver.hpp>
#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Parser/Context.hpp>

#include "LexerAPI.hpp"
#include "LocationInfo.hpp"
#include "Token.hpp"

namespace locic {
	
	namespace Parser {
		
		int ConvertTokenKind(const Lex::Token::Kind kind) {
			switch (kind) {
				case Lex::Token::NAME:
					return NAME;
				case Lex::Token::VERSION:
					return VERSION;
				case Lex::Token::CONSTANT:
					return CONSTANT;
				case Lex::Token::TRUEVAL:
					return TRUEVAL;
				case Lex::Token::FALSEVAL:
					return FALSEVAL;
				case Lex::Token::UNKNOWN:
					return UNKNOWN;
				case Lex::Token::ERROR:
					return ERROR;
				case Lex::Token::INTERFACE:
					return INTERFACE;
				case Lex::Token::SEMICOLON:
					return SEMICOLON;
				case Lex::Token::NAMESPACE:
					return NAMESPACE;
				case Lex::Token::UNDERSCORE:
					return UNDERSCORE;
				case Lex::Token::LCURLYBRACKET:
					return LCURLYBRACKET;
				case Lex::Token::RCURLYBRACKET:
					return RCURLYBRACKET;
				case Lex::Token::LSQUAREBRACKET:
					return LSQUAREBRACKET;
				case Lex::Token::RSQUAREBRACKET:
					return RSQUAREBRACKET;
				case Lex::Token::LROUNDBRACKET:
					return LROUNDBRACKET;
				case Lex::Token::RROUNDBRACKET:
					return RROUNDBRACKET;
				case Lex::Token::DOUBLE_LTRIBRACKET:
					return DOUBLE_LTRIBRACKET;
				case Lex::Token::LTRIBRACKET:
					return LTRIBRACKET;
				case Lex::Token::RTRIBRACKET:
					return RTRIBRACKET;
				case Lex::Token::AUTO:
					return AUTO;
				case Lex::Token::STATIC:
					return STATIC;
				case Lex::Token::IMPORT:
					return IMPORT;
				case Lex::Token::EXPORT:
					return EXPORT;
				case Lex::Token::MOVE:
					return MOVE;
				case Lex::Token::LVAL:
					return LVAL;
				case Lex::Token::NOLVAL:
					return NOLVAL;
				case Lex::Token::REF:
					return REF;
				case Lex::Token::STATICREF:
					return STATICREF;
				case Lex::Token::NOREF:
					return NOREF;
				case Lex::Token::NOTAG:
					return NOTAG;
				case Lex::Token::TEMPLATE:
					return TEMPLATE;
				case Lex::Token::TYPENAME:
					return TYPENAME;
				case Lex::Token::VIRTUAL:
					return VIRTUAL;
				case Lex::Token::REQUIRE:
					return REQUIRE;
				case Lex::Token::UNUSED:
					return UNUSED;
				case Lex::Token::UNUSED_RESULT:
					return UNUSED_RESULT;
				case Lex::Token::USING:
					return USING;
				case Lex::Token::LET:
					return LET;
				case Lex::Token::ENUM:
					return ENUM;
				case Lex::Token::UNION:
					return UNION;
				case Lex::Token::CASE:
					return CASE;
				case Lex::Token::SWITCH:
					return SWITCH;
				case Lex::Token::DEFAULT:
					return DEFAULT;
				case Lex::Token::CONTINUE:
					return CONTINUE;
				case Lex::Token::BREAK:
					return BREAK;
				case Lex::Token::EXCEPTION:
					return EXCEPTION;
				case Lex::Token::THROW:
					return THROW;
				case Lex::Token::TRY:
					return TRY;
				case Lex::Token::CATCH:
					return CATCH;
				case Lex::Token::SCOPE:
					return SCOPE;
				case Lex::Token::NOEXCEPT:
					return NOEXCEPT;
				case Lex::Token::ALIGNOF:
					return ALIGNOF;
				case Lex::Token::SIZEOF:
					return SIZEOF;
				case Lex::Token::TYPEOF:
					return TYPEOF;
				case Lex::Token::TYPEID:
					return TYPEID;
				case Lex::Token::PRIMITIVE:
					return PRIMITIVE;
				case Lex::Token::PRIMITIVEFUNCTION:
					return PRIMITIVEFUNCTION;
				case Lex::Token::STRUCT:
					return STRUCT;
				case Lex::Token::CLASS:
					return CLASS;
				case Lex::Token::DATATYPE:
					return DATATYPE;
				case Lex::Token::SIGNED:
					return SIGNED;
				case Lex::Token::UNSIGNED:
					return UNSIGNED;
				case Lex::Token::BYTE:
					return BYTE;
				case Lex::Token::UBYTE:
					return UBYTE;
				case Lex::Token::SHORT:
					return SHORT;
				case Lex::Token::USHORT:
					return USHORT;
				case Lex::Token::INT:
					return INT;
				case Lex::Token::UINT:
					return UINT;
				case Lex::Token::LONG:
					return LONG;
				case Lex::Token::ULONG:
					return ULONG;
				case Lex::Token::LONGLONG:
					return LONGLONG;
				case Lex::Token::ULONGLONG:
					return ULONGLONG;
				case Lex::Token::FLOAT:
					return FLOAT;
				case Lex::Token::DOUBLE:
					return DOUBLE;
				case Lex::Token::COLON:
					return COLON;
				case Lex::Token::DOUBLE_COLON:
					return DOUBLE_COLON;
				case Lex::Token::VOID:
					return VOID;
				case Lex::Token::BOOL:
					return BOOL;
				case Lex::Token::FINAL:
					return FINAL;
				case Lex::Token::CONST:
					return CONST;
				case Lex::Token::MUTABLE:
					return MUTABLE;
				case Lex::Token::OVERRIDE_CONST:
					return OVERRIDE_CONST;
				case Lex::Token::STAR:
					return STAR;
				case Lex::Token::COMMA:
					return COMMA;
				case Lex::Token::IF:
					return IF;
				case Lex::Token::ELSE:
					return ELSE;
				case Lex::Token::FOR:
					return FOR;
				case Lex::Token::WHILE:
					return WHILE;
				case Lex::Token::SETEQUAL:
					return SETEQUAL;
				case Lex::Token::ADDEQUAL:
					return ADDEQUAL;
				case Lex::Token::SUBEQUAL:
					return SUBEQUAL;
				case Lex::Token::MULEQUAL:
					return MULEQUAL;
				case Lex::Token::DIVEQUAL:
					return DIVEQUAL;
				case Lex::Token::PERCENTEQUAL:
					return PERCENTEQUAL;
				case Lex::Token::RETURN:
					return RETURN;
				case Lex::Token::ASSERT:
					return ASSERT;
				case Lex::Token::UNREACHABLE:
					return UNREACHABLE;
				case Lex::Token::AT:
					return AT;
				case Lex::Token::NULLVAL:
					return NULLVAL;
				case Lex::Token::CONST_CAST:
					return CONST_CAST;
				case Lex::Token::DYNAMIC_CAST:
					return DYNAMIC_CAST;
				case Lex::Token::REINTERPRET_CAST:
					return REINTERPRET_CAST;
				case Lex::Token::AND:
					return AND;
				case Lex::Token::OR:
					return OR;
				case Lex::Token::IS_A:
					return IS_A;
				case Lex::Token::DOT:
					return DOT;
				case Lex::Token::PTRACCESS:
					return PTRACCESS;
				case Lex::Token::PLUS:
					return PLUS;
				case Lex::Token::DOUBLE_PLUS:
					return DOUBLE_PLUS;
				case Lex::Token::MINUS:
					return MINUS;
				case Lex::Token::DOUBLE_MINUS:
					return DOUBLE_MINUS;
				case Lex::Token::EXCLAIMMARK:
					return EXCLAIMMARK;
				case Lex::Token::AMPERSAND:
					return AMPERSAND;
				case Lex::Token::DOUBLE_AMPERSAND:
					return DOUBLE_AMPERSAND;
				case Lex::Token::VERTICAL_BAR:
					return VERTICAL_BAR;
				case Lex::Token::DOUBLE_VERTICAL_BAR:
					return DOUBLE_VERTICAL_BAR;
				case Lex::Token::FORWARDSLASH:
					return FORWARDSLASH;
				case Lex::Token::PERCENT:
					return PERCENT;
				case Lex::Token::ISEQUAL:
					return ISEQUAL;
				case Lex::Token::NOTEQUAL:
					return NOTEQUAL;
				case Lex::Token::GREATEROREQUAL:
					return GREATEROREQUAL;
				case Lex::Token::LESSOREQUAL:
					return LESSOREQUAL;
				case Lex::Token::QUESTIONMARK:
					return QUESTIONMARK;
				case Lex::Token::TILDA:
					return TILDA;
				case Lex::Token::SELF:
					return SELF;
				case Lex::Token::THIS:
					return THIS;
			}
		}
		
		GeneratedToken ConvertToken(const Lex::Token& token) {
			GeneratedToken newToken;
			switch (token.kind()) {
				case Lex::Token::CONSTANT: {
					newToken.lexer_constant = token.constant();
					break;
				}
				case Lex::Token::NAME: {
					newToken.lexer_str = token.name();
					break;
				}
				case Lex::Token::VERSION: {
					newToken.lexer_version = new Version(token.version());
					break;
				}
				default:
					assert(!token.hasAssociatedData());
					break;
			}
			return newToken;
		}
		
		LocationInfo ConvertRange(const Debug::SourceRange& sourceRange) {
			LocationInfo locationInfo;
			locationInfo.first_line = sourceRange.start().lineNumber();
			locationInfo.last_line = sourceRange.end().lineNumber();
			locationInfo.first_column = sourceRange.start().column();
			locationInfo.last_column = sourceRange.end().column();
			locationInfo.first_byte = sourceRange.start().byteOffset();
			locationInfo.last_byte = sourceRange.end().byteOffset();
			return locationInfo;
		}
		
		class LexLexer: public LexerAPI, public Lex::DiagnosticReceiver, public Lex::CharacterSource {
		public:
			LexLexer(FILE * file, Context& context)
			: file_(file), context_(context),
			position_(0), lexer_(*this, *this) { }
			
			~LexLexer() { }
			
			LexLexer(const LexLexer&) = delete;
			LexLexer& operator=(const LexLexer&) = delete;
			
			LexLexer(LexLexer&&) = delete;
			LexLexer& operator=(LexLexer&&) = delete;
			
			Lex::Character get() {
				const auto result = fgetc(file_);
				if (feof(file_)) {
					return Lex::Character(0);
				}
				position_++;
				return Lex::Character(result);
			}
			
			size_t byteOffset() const {
				return position_;
			}
			
			void issueWarning(Lex::Diag /*kind*/, Debug::SourceRange range) {
				printf("Warning at %s\n", range.toString().c_str());
			}
			
			void issueError(Lex::Diag /*kind*/, Debug::SourceRange range) {
				printf("Error at %s\n", range.toString().c_str());
			}
			
			int getToken(GeneratedToken* tokenPtr, LocationInfo* position) {
				const auto token = lexer_.lexToken(context_.stringHost());
				if (token) {
					const auto result = ConvertTokenKind(token->kind());
					*tokenPtr = ConvertToken(*token);
					*position = ConvertRange(token->sourceRange());
					return result;
				} else {
					return 0;
				}
			}
			
		private:
			FILE * file_;
			Context& context_;
			size_t position_;
			Lex::Lexer lexer_;
			
		};
		
	}
	
}

#endif
