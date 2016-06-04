#include <locic/Lex/Character.hpp>
#include <locic/Lex/CharacterReader.hpp>
#include <locic/Lex/CharacterSource.hpp>
#include <locic/Lex/IdentifierLexer.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringBuilder.hpp>
#include <locic/Support/StringHost.hpp>

namespace locic {
	
	namespace Lex {
		
		IdentifierLexer::IdentifierLexer(CharacterReader& reader,
		                                 const StringHost& stringHost)
		: reader_(reader), stringHost_(stringHost) { }
		
		IdentifierLexer::~IdentifierLexer() { }
		
		Character IdentifierLexer::get() {
			const auto value = reader_.peek();
			if (!value.isAlpha() && !value.isDigit() && value != '_') {
				return Character(0);
			}
			
			savedValues_.push_back(value);
			reader_.consume();
			return value;
		}
		
		bool IdentifierLexer::lexCommonPrefix(const char* const prefix) {
			size_t position = savedValues_.size();
			
			while (true) {
				if (prefix[position] == 0) {
					return true;
				}
				
				const auto value = get();
				if (value != prefix[position]) {
					return false;
				}
				
				position++;
			}
		}
		
		Token IdentifierLexer::lexPossibleKeyword(const char* const name,
		                                          const Token::Kind tokenKind) {
			size_t position = savedValues_.size();
			
			while (true) {
				const auto value = get();
				if (value != name[position]) {
					break;
				}
				
				if (value == 0) {
					return Token::Basic(tokenKind);
				}
				
				position++;
			}
			
			return lexGeneralIdentifier();
		}
		
		Token IdentifierLexer::lexGeneralIdentifier() {
			while (true) {
				const auto value = get();
				
				if (value == 0) {
					break;
				}
			}
			
			StringBuilder stringLiteral(stringHost_);
			stringLiteral.reserve(16);
			
			for (const auto c: savedValues_) {
				stringLiteral.append(c.asciiValue());
			}
			
			return Token::Name(stringLiteral.getString());
		}
		
		Token IdentifierLexer::lexIdentifier() {
			savedValues_.clear();
			return lexPrefix();
		}
		
		Token IdentifierLexer::lexPrefix() {
			switch (get().value()) {
				case '_':
					return lexPrefix_();
				case 'a':
					return lexPrefixA();
				case 'b':
					return lexPrefixB();
				case 'c':
					return lexPrefixC();
				case 'd':
					return lexPrefixD();
				case 'e':
					return lexPrefixE();
				case 'f':
					return lexPrefixF();
				case 'i':
					return lexPrefixI();
				case 'l':
					return lexPrefixL();
				case 'm':
					return lexPrefixM();
				case 'n':
					return lexPrefixN();
				case 'o':
					return lexPossibleKeyword("or", Token::OR);
				case 'r':
					return lexPrefixR();
				case 's':
					return lexPrefixS();
				case 't':
					return lexPrefixT();
				case 'u':
					return lexPrefixU();
				case 'v':
					return lexPrefixV();
				case 'w':
					return lexPossibleKeyword("while", Token::WHILE);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefix_() {
			switch (get().value()) {
				case 0:
					return Token::Basic(Token::UNDERSCORE);
				case '_':
					return lexPrefix__();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefix__() {
			switch (get().value()) {
				case 'o':
					return lexPossibleKeyword("__override_const", Token::OVERRIDE_CONST);
				case 'p':
					return lexPrefix__p();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefix__p() {
			const std::string c = "rimitive";
			for (size_t i = 0; i < c.size(); i++) {
				if (get() != c[i]) {
					return lexGeneralIdentifier();
				}
			}
			
			switch (get().value()) {
				case 0:
					return lexPossibleKeyword("__primitive", Token::PRIMITIVE);
				case 'f':
					return lexPossibleKeyword("__primitivefunction", Token::PRIMITIVEFUNCTION);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixA() {
			switch (get().value()) {
				case 'l':
					return lexPossibleKeyword("alignof", Token::ALIGNOF);
				case 'n':
					return lexPossibleKeyword("and", Token::AND);
				case 's':
					return lexPossibleKeyword("assert", Token::ASSERT);
				case 'u':
					return lexPossibleKeyword("auto", Token::AUTO);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixB() {
			switch (get().value()) {
				case 'o':
					return lexPossibleKeyword("bool", Token::BOOL);
				case 'r':
					return lexPossibleKeyword("break", Token::BREAK);
				case 'y':
					return lexPossibleKeyword("byte", Token::BYTE);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixC() {
			switch (get().value()) {
				case 'a':
					return lexPrefixCa();
				case 'l':
					return lexPossibleKeyword("class", Token::CLASS);
				case 'o':
					return lexPrefixCo();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixCa() {
			switch (get().value()) {
				case 's':
					return lexPossibleKeyword("case", Token::CASE);
				case 't':
					return lexPossibleKeyword("catch", Token::CATCH);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixCo() {
			switch (get().value()) {
				case 'n':
					return lexPrefixCon();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixCon() {
			switch (get().value()) {
				case 's':
					return lexPrefixCons();
				case 't':
					return lexPossibleKeyword("continue", Token::CONTINUE);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixCons() {
			switch (get().value()) {
				case 't':
					return lexPrefixConst();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixConst() {
			switch (get().value()) {
				case 0:
					return Token::Basic(Token::CONST);
				case '_':
					return lexPossibleKeyword("const_cast", Token::CONST_CAST);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixD() {
			switch (get().value()) {
				case 'a':
					return lexPossibleKeyword("datatype", Token::DATATYPE);
				case 'e':
					return lexPossibleKeyword("default", Token::DEFAULT);
				case 'o':
					return lexPossibleKeyword("double", Token::DOUBLE);
				case 'y':
					return lexPossibleKeyword("dynamic_cast", Token::DYNAMIC_CAST);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixE() {
			switch (get().value()) {
				case 'l':
					return lexPossibleKeyword("else", Token::ELSE);
				case 'n':
					return lexPossibleKeyword("enum", Token::ENUM);
				case 'x':
					return lexPrefixEx();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixEx() {
			switch (get().value()) {
				case 'c':
					return lexPossibleKeyword("exception", Token::EXCEPTION);
				case 'p':
					return lexPossibleKeyword("export", Token::EXPORT);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixF() {
			switch (get().value()) {
				case 'a':
					return lexPossibleKeyword("false", Token::FALSEVAL);
				case 'i':
					return lexPossibleKeyword("final", Token::FINAL);
				case 'l':
					return lexPossibleKeyword("float", Token::FLOAT);
				case 'o':
					return lexPossibleKeyword("for", Token::FOR);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixI() {
			switch (get().value()) {
				case 'f':
					return lexPossibleKeyword("if", Token::IF);
				case 'n':
					return lexPrefixIn();
				case 'm':
					return lexPossibleKeyword("import", Token::IMPORT);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixIn() {
			switch (get().value()) {
				case 'h':
					return lexPossibleKeyword("inherit", Token::INHERIT);
				case 't':
					return lexPrefixInt();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixInt() {
			switch (get().value()) {
				case 0:
					return lexPossibleKeyword("int", Token::INT);
				case 'e':
					return lexPossibleKeyword("interface", Token::INTERFACE);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixL() {
			switch (get().value()) {
				case 'e':
					return lexPossibleKeyword("let", Token::LET);
				case 'o':
					return lexPrefixLo();
				case 'v':
					return lexPossibleKeyword("lval", Token::LVAL);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixLo() {
			if (lexCommonPrefix("long")) {
				return lexPrefixLong();
			} else {
				return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixLong() {
			switch (get().value()) {
				case 0:
					return Token::Basic(Token::LONG);
				case 'l':
					return lexPossibleKeyword("longlong", Token::LONGLONG);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixM() {
			switch (get().value()) {
				case 'o':
					return lexPossibleKeyword("move", Token::MOVE);
				case 'u':
					return lexPossibleKeyword("mutable", Token::MUTABLE);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixN() {
			switch (get().value()) {
				case 'a':
					return lexPossibleKeyword("namespace", Token::NAMESPACE);
				case 'o':
					return lexPrefixNo();
				case 'u':
					return lexPossibleKeyword("null", Token::NULLVAL);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixNo() {
			switch (get().value()) {
				case 'e':
					return lexPossibleKeyword("noexcept", Token::NOEXCEPT);
				case 'l':
					return lexPossibleKeyword("nolval", Token::NOLVAL);
				case 'r':
					return lexPossibleKeyword("noref", Token::NOREF);
				case 't':
					return lexPossibleKeyword("notag", Token::NOTAG);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixR() {
			switch (get().value()) {
				case 'e':
					return lexPrefixRe();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixRe() {
			switch (get().value()) {
				case 'f':
					return lexPossibleKeyword("ref", Token::REF);
				case 'i':
					return lexPossibleKeyword("reinterpret_cast", Token::REINTERPRET_CAST);
				case 'q':
					return lexPossibleKeyword("require", Token::REQUIRE);
				case 't':
					return lexPossibleKeyword("return", Token::RETURN);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixS() {
			switch (get().value()) {
				case 'c':
					return lexPossibleKeyword("scope", Token::SCOPE);
				case 'e':
					return lexPossibleKeyword("self", Token::SELF);
				case 'h':
					return lexPossibleKeyword("short", Token::SHORT);
				case 'i':
					return lexPrefixSi();
				case 't':
					return lexPrefixSt();
				case 'w':
					return lexPossibleKeyword("switch", Token::SWITCH);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixSi() {
			switch (get().value()) {
				case 'g':
					return lexPossibleKeyword("signed", Token::SIGNED);
				case 'z':
					return lexPossibleKeyword("sizeof", Token::SIZEOF);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixSt() {
			switch (get().value()) {
				case 'a':
					return lexPrefixSta();
				case 'r':
					return lexPossibleKeyword("struct", Token::STRUCT);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixSta() {
			switch (get().value()) {
				case 't':
					return lexPrefixStat();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixStat() {
			switch (get().value()) {
				case 'i':
					return lexPrefixStati();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixStati() {
			switch (get().value()) {
				case 'c':
					return lexPrefixStatic();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixStatic() {
			switch (get().value()) {
				case 0:
					return lexPossibleKeyword("static", Token::STATIC);
				case 'r':
					return lexPossibleKeyword("staticref", Token::STATICREF);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixT() {
			switch (get().value()) {
				case 'e':
					return lexPossibleKeyword("template", Token::TEMPLATE);
				case 'h':
					return lexPrefixTh();
				case 'r':
					return lexPrefixTr();
				case 'y':
					return lexPrefixTy();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixTh() {
			switch (get().value()) {
				case 'i':
					return lexPossibleKeyword("this", Token::THIS);
				case 'r':
					return lexPossibleKeyword("throw", Token::THROW);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixTr() {
			switch (get().value()) {
				case 'u':
					return lexPossibleKeyword("true", Token::TRUEVAL);
				case 'y':
					return lexPossibleKeyword("try", Token::TRY);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixTy() {
			switch (get().value()) {
				case 'p':
					return lexPrefixTyp();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixTyp() {
			switch (get().value()) {
				case 'e':
					return lexPrefixType();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixType() {
			switch (get().value()) {
				case 'i':
					return lexPossibleKeyword("typeid", Token::TYPEID);
				case 'n':
					return lexPossibleKeyword("typename", Token::TYPENAME);
				case 'o':
					return lexPossibleKeyword("typeof", Token::TYPEOF);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixU() {
			switch (get().value()) {
				case 'b':
					return lexPossibleKeyword("ubyte", Token::UBYTE);
				case 'i':
					return lexPossibleKeyword("uint", Token::UINT);
				case 'l':
					return lexPrefixUl();
				case 'n':
					return lexPrefixUn();
				case 's':
					return lexPrefixUs();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixUl() {
			if (lexCommonPrefix("ulong")) {
				return lexPrefixUlong();
			} else {
				return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixUlong() {
			switch (get().value()) {
				case 0:
					return Token::Basic(Token::ULONG);
				case 'l':
					return lexPossibleKeyword("ulonglong", Token::ULONGLONG);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixUn() {
			switch (get().value()) {
				case 'i':
					return lexPrefixUni();
				case 'r':
					return lexPossibleKeyword("unreachable", Token::UNREACHABLE);
				case 's':
					return lexPossibleKeyword("unsigned", Token::UNSIGNED);
				case 'u':
					return lexPrefixUnu();
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixUni() {
			switch (get().value()) {
				case 'c':
					return lexPossibleKeyword("unichar", Token::UNICHAR);
				case 'o':
					return lexPossibleKeyword("union", Token::UNION);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixUnu() {
			if (lexCommonPrefix("unused")) {
				return lexPrefixUnused();
			} else {
				return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixUnused() {
			switch (get().value()) {
				case 0:
					return lexPossibleKeyword("unused", Token::UNUSED);
				case '_':
					return lexPossibleKeyword("unused_result", Token::UNUSED_RESULT);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixUs() {
			switch (get().value()) {
				case 'h':
					return lexPossibleKeyword("ushort", Token::USHORT);
				case 'i':
					return lexPossibleKeyword("using", Token::USING);
				default:
					return lexGeneralIdentifier();
			}
		}
		
		Token IdentifierLexer::lexPrefixV() {
			switch (get().value()) {
				case 'i':
					return lexPossibleKeyword("virtual", Token::VIRTUAL);
				case 'o':
					return lexPossibleKeyword("void", Token::VOID);
				default:
					return lexGeneralIdentifier();
			}
		}
		
	}
	
}
