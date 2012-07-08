%{

#include <string>
#include <Locic/AST.hpp>
#include <Locic/LexerContext.hpp>
#include <Locic/Parser.h>
#include <Locic/Token.hpp>

#define YY_EXTRA_TYPE Locic::LexerContext *
#define YY_USER_ACTION yyextra->lineNumber = yylineno;

/*
"\""                    { strText = ""; BEGIN(STRING_STATE); }

<STRING_STATE>"\\\""          { strText += "\""; }
<STRING_STATE>"\\\\"          { strText += "\\"; }
<STRING_STATE>"\\n"           { strText += "\n"; }
<STRING_STATE>"\\t"           { strText += "\t"; }
<STRING_STATE>"\""            { token.value = new String(strText);
				BEGIN(INITIAL);
				return STRING; }
<STRING_STATE>.               { strText += yytext; }
*/

//%x STRING_STATE

%}

%option prefix="Locic_GeneratedLexer_"
%option reentrant
%option noyywrap
%option yylineno

%x COMMENT_STATE

%%

"/*"                    { BEGIN(COMMENT_STATE); }
<COMMENT_STATE>[^*\n]*        { }
<COMMENT_STATE>"*"+[^*/\n]*   { }
<COMMENT_STATE>\n             { }
<COMMENT_STATE>"*"+"/"        { BEGIN(INITIAL); }

if                      { return LOCIC_TOKEN_IF; }
else                    { return LOCIC_TOKEN_ELSE; }
while                   { return LOCIC_TOKEN_WHILE; }
for                     { return LOCIC_TOKEN_FOR; }
return                  { return LOCIC_TOKEN_RETURN; }

class                   { return LOCIC_TOKEN_CLASS; }
interface               { return LOCIC_TOKEN_INTERFACE; }
struct                  { return LOCIC_TOKEN_STRUCT; }

auto                    { return LOCIC_TOKEN_AUTO; }
const                   { return LOCIC_TOKEN_CONST; }
cast                    { return LOCIC_TOKEN_CAST; }

void                    { return LOCIC_TOKEN_VOIDNAME; }
int                     { return LOCIC_TOKEN_INTNAME; }
bool                    { return LOCIC_TOKEN_BOOLNAME; }
float                   { return LOCIC_TOKEN_FLOATNAME; }

null                    { return LOCIC_TOKEN_NULL; }
true                    { yyextra->token.boolValue = 1; return LOCIC_TOKEN_BOOLCONSTANT; }
false                   { yyextra->token.boolValue = 0; return LOCIC_TOKEN_BOOLCONSTANT; }
[0-9]+                  { yyextra->token.intValue = atoi(yytext); return LOCIC_TOKEN_INTCONSTANT; }
[0-9]+\.[0-9]+          { yyextra->token.floatValue = atof(yytext); return LOCIC_TOKEN_FLOATCONSTANT; }

[A-Z]((_[A-Za-z0-9])|[A-Za-z0-9])*       { yyextra->token.str = new std::string(yytext); return LOCIC_TOKEN_UCNAME; }
[a-z]((_[A-Za-z0-9])|[A-Za-z0-9])*      { yyextra->token.str = new std::string(yytext); return LOCIC_TOKEN_LCNAME; }

"=="                    { return LOCIC_TOKEN_ISEQUAL; }
"!="                    { return LOCIC_TOKEN_NOTEQUAL; }
">="                    { return LOCIC_TOKEN_GREATEROREQUAL; }
"<="                    { return LOCIC_TOKEN_LESSOREQUAL; }
"->"                    { return LOCIC_TOKEN_PTRACCESS; }

"+="                    { return LOCIC_TOKEN_ADDEQUAL; }
"-="                    { return LOCIC_TOKEN_SUBEQUAL; }
"*="                    { return LOCIC_TOKEN_MULEQUAL; }
"/="                    { return LOCIC_TOKEN_DIVEQUAL; }

\/\/(.*?)\n             { }

"%"                     { return LOCIC_TOKEN_PERCENT; }
"="                     { return LOCIC_TOKEN_SETEQUAL; }
"+"                     { return LOCIC_TOKEN_PLUS; }
"-"                     { return LOCIC_TOKEN_MINUS; }
"*"                     { return LOCIC_TOKEN_STAR; }
"/"                     { return LOCIC_TOKEN_FORWARDSLASH; }
"!"                     { return LOCIC_TOKEN_EXCLAIMMARK; }
"&"                     { return LOCIC_TOKEN_AMPERSAND; }
"?"                     { return LOCIC_TOKEN_QUESTIONMARK; }
"@"                     { return LOCIC_TOKEN_AT; }
","                     { return LOCIC_TOKEN_COMMA; }
":"                     { return LOCIC_TOKEN_COLON; }
";"                     { return LOCIC_TOKEN_SEMICOLON; }
"("                     { return LOCIC_TOKEN_LROUNDBRACKET; }
")"                     { return LOCIC_TOKEN_RROUNDBRACKET; }
"{"                     { return LOCIC_TOKEN_LCURLYBRACKET; }
"}"                     { return LOCIC_TOKEN_RCURLYBRACKET; }
"."                     { return LOCIC_TOKEN_DOT; }
"<"                     { return LOCIC_TOKEN_LTRIBRACKET; }
">"                     { return LOCIC_TOKEN_RTRIBRACKET; }
\n                      { }
[ \t\r]+                { }
.                       { return LOCIC_TOKEN_ERROR; }

%%

void * Locic_LexAlloc(FILE * file, Locic::LexerContext * context){
	void * lexer;
	Locic_GeneratedLexer_lex_init(&lexer);
	Locic_GeneratedLexer_set_in(file, lexer);
	Locic_GeneratedLexer_set_extra(context, lexer);
	return lexer;
}

int Locic_Lex(void * lexer){
	return Locic_GeneratedLexer_lex(lexer);
}

void Locic_LexFree(void * lexer){
	Locic_GeneratedLexer_lex_destroy(lexer);
}

