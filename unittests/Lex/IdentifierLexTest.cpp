#include "gtest/gtest.h"

#include <locic/Lex/Lexer.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/StringHost.hpp>

#include "LexTest.hpp"
#include "MockCharacterSource.hpp"
#include "MockDiagnosticReceiver.hpp"

void testKeyword(const std::string& text, const locic::Lex::Token::Kind result) {
	const auto start = locic::Debug::SourcePosition(/*lineNumber=*/1, /*column=*/1,
	                                         /*byteOffset=*/0);
	const auto end = locic::Debug::SourcePosition(/*lineNumber=*/1, /*column=*/text.size() + 1,
	                                       /*byteOffset=*/text.size());
	const auto range = locic::Debug::SourceRange(start, end);
	testLexer(text, { locic::Lex::Token::Basic(result, range) }, /*diags=*/{});
}

TEST(IdentifierLexTest, Keywords) {
	testKeyword("__primitive", locic::Lex::Token::Kind::PRIMITIVE);
	testKeyword("__primitivefunction", locic::Lex::Token::Kind::PRIMITIVEFUNCTION);
	
	testKeyword("alignof", locic::Lex::Token::Kind::ALIGNOF);
	testKeyword("and", locic::Lex::Token::Kind::AND);
	testKeyword("assert", locic::Lex::Token::Kind::ASSERT);
	testKeyword("auto", locic::Lex::Token::Kind::AUTO);
	
	testKeyword("bool", locic::Lex::Token::Kind::BOOL);
	testKeyword("break", locic::Lex::Token::Kind::BREAK);
	testKeyword("byte", locic::Lex::Token::Kind::BYTE);
	
	testKeyword("case", locic::Lex::Token::Kind::CASE);
	testKeyword("catch", locic::Lex::Token::Kind::CATCH);
	testKeyword("class", locic::Lex::Token::Kind::CLASS);
	testKeyword("const", locic::Lex::Token::Kind::CONST);
	testKeyword("const_cast", locic::Lex::Token::Kind::CONST_CAST);
	testKeyword("continue", locic::Lex::Token::Kind::CONTINUE);
	
	testKeyword("datatype", locic::Lex::Token::Kind::DATATYPE);
	testKeyword("default", locic::Lex::Token::Kind::DEFAULT);
	testKeyword("double", locic::Lex::Token::Kind::DOUBLE);
	testKeyword("dynamic_cast", locic::Lex::Token::Kind::DYNAMIC_CAST);
	
	testKeyword("else", locic::Lex::Token::Kind::ELSE);
	testKeyword("enum", locic::Lex::Token::Kind::ENUM);
	testKeyword("exception", locic::Lex::Token::Kind::EXCEPTION);
	testKeyword("export", locic::Lex::Token::Kind::EXPORT);
	
	testKeyword("false", locic::Lex::Token::Kind::FALSEVAL);
	testKeyword("float", locic::Lex::Token::Kind::FLOAT);
	testKeyword("for", locic::Lex::Token::Kind::FOR);
	
	testKeyword("if", locic::Lex::Token::Kind::IF);
	testKeyword("inherit", locic::Lex::Token::Kind::INHERIT);
	testKeyword("int", locic::Lex::Token::Kind::INT);
	testKeyword("interface", locic::Lex::Token::Kind::INTERFACE);
	testKeyword("import", locic::Lex::Token::Kind::IMPORT);
	
	testKeyword("let", locic::Lex::Token::Kind::LET);
	testKeyword("long", locic::Lex::Token::Kind::LONG);
	testKeyword("longlong", locic::Lex::Token::Kind::LONGLONG);
	
	testKeyword("move", locic::Lex::Token::Kind::MOVE);
	testKeyword("mutable", locic::Lex::Token::Kind::MUTABLE);
	
	testKeyword("namespace", locic::Lex::Token::Kind::NAMESPACE);
	testKeyword("new", locic::Lex::Token::Kind::NEW);
	testKeyword("noexcept", locic::Lex::Token::Kind::NOEXCEPT);
	testKeyword("noref", locic::Lex::Token::Kind::NOREF);
	testKeyword("notag", locic::Lex::Token::Kind::NOTAG);
	
	testKeyword("or", locic::Lex::Token::Kind::OR);
	testKeyword("override", locic::Lex::Token::Kind::OVERRIDE);
	
	testKeyword("ref", locic::Lex::Token::Kind::REF);
	testKeyword("reinterpret_cast", locic::Lex::Token::Kind::REINTERPRET_CAST);
	testKeyword("require", locic::Lex::Token::Kind::REQUIRE);
	testKeyword("return", locic::Lex::Token::Kind::RETURN);
	
	testKeyword("scope", locic::Lex::Token::Kind::SCOPE);
	testKeyword("self", locic::Lex::Token::Kind::SELF);
	testKeyword("short", locic::Lex::Token::Kind::SHORT);
	testKeyword("signed", locic::Lex::Token::Kind::SIGNED);
	testKeyword("sizeof", locic::Lex::Token::Kind::SIZEOF);
	testKeyword("static", locic::Lex::Token::Kind::STATIC);
	testKeyword("staticref", locic::Lex::Token::Kind::STATICREF);
	testKeyword("struct", locic::Lex::Token::Kind::STRUCT);
	testKeyword("switch", locic::Lex::Token::Kind::SWITCH);
	
	testKeyword("template", locic::Lex::Token::Kind::TEMPLATE);
	testKeyword("this", locic::Lex::Token::Kind::THIS);
	testKeyword("throw", locic::Lex::Token::Kind::THROW);
	testKeyword("true", locic::Lex::Token::Kind::TRUEVAL);
	testKeyword("try", locic::Lex::Token::Kind::TRY);
	testKeyword("typeid", locic::Lex::Token::Kind::TYPEID);
	testKeyword("typename", locic::Lex::Token::Kind::TYPENAME);
	testKeyword("typeof", locic::Lex::Token::Kind::TYPEOF);
	
	testKeyword("ubyte", locic::Lex::Token::Kind::UBYTE);
	testKeyword("uint", locic::Lex::Token::Kind::UINT);
	testKeyword("ulong", locic::Lex::Token::Kind::ULONG);
	testKeyword("ulonglong", locic::Lex::Token::Kind::ULONGLONG);
	testKeyword("unichar", locic::Lex::Token::Kind::UNICHAR);
	testKeyword("union", locic::Lex::Token::Kind::UNION);
	testKeyword("unreachable", locic::Lex::Token::Kind::UNREACHABLE);
	testKeyword("unsigned", locic::Lex::Token::Kind::UNSIGNED);
	testKeyword("unused", locic::Lex::Token::Kind::UNUSED);
	testKeyword("unused_result", locic::Lex::Token::Kind::UNUSED_RESULT);
	testKeyword("ushort", locic::Lex::Token::Kind::USHORT);
	testKeyword("using", locic::Lex::Token::Kind::USING);
	
	testKeyword("virtual", locic::Lex::Token::Kind::VIRTUAL);
	testKeyword("void", locic::Lex::Token::Kind::VOID);
}
