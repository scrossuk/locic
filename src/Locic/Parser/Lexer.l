%{

#include <string>
#include <Locic/AST.hpp>
#include <Locic/Parser/Context.hpp>
#include <Locic/Parser/Lexer.hpp>
#include <Locic/Parser/Token.hpp>

#define YY_EXTRA_TYPE Locic::Parser::Context *
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

%option prefix="Locic_Parser_GeneratedLexer_"
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

if                      { return IF; }
else                    { return ELSE; }
while                   { return WHILE; }
for                     { return FOR; }
return                  { return RETURN; }

class                   { return CLASS; }
interface               { return INTERFACE; }
struct                 { return STRUCT; }
namespace               { return NAMESPACE; }

auto                    { return AUTO; }
const                   { return CONST; }
cast                    { return CAST; }
static                 { return STATIC; }

void                    { return VOIDNAME; }
int                     { return INTNAME; }
bool                    { return BOOLNAME; }
float                   { return FLOATNAME; }

null                    { return NULLVAL; }
true                    { yyextra->token.boolValue = true; return BOOLCONSTANT; }
false                   { yyextra->token.boolValue = false; return BOOLCONSTANT; }
[0-9]+                  { yyextra->token.intValue = atoi(yytext); return INTCONSTANT; }
[0-9]+\.[0-9]+          { yyextra->token.floatValue = atof(yytext); return FLOATCONSTANT; }

[A-Za-z_][A-Za-z0-9_]* { yyextra->token.str = new std::string(yytext); return NAME; }

"=="                    { return ISEQUAL; }
"!="                    { return NOTEQUAL; }
">="                    { return GREATEROREQUAL; }
"<="                    { return LESSOREQUAL; }
"->"                    { return PTRACCESS; }

"+="                    { return ADDEQUAL; }
"-="                    { return SUBEQUAL; }
"*="                    { return MULEQUAL; }
"/="                    { return DIVEQUAL; }

\/\/(.*?)\n             { }

"%"                     { return PERCENT; }
"="                     { return SETEQUAL; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return STAR; }
"/"                     { return FORWARDSLASH; }
"!"                     { return EXCLAIMMARK; }
"&"                     { return AMPERSAND; }
"?"                     { return QUESTIONMARK; }
"@"                     { return AT; }
","                     { return COMMA; }
":"                     { return COLON; }
";"                     { return SEMICOLON; }
"("                     { return LROUNDBRACKET; }
")"                     { return RROUNDBRACKET; }
"{"                     { return LCURLYBRACKET; }
"}"                     { return RCURLYBRACKET; }
"."                     { return DOT; }
"<"                     { return LTRIBRACKET; }
">"                     { return RTRIBRACKET; }
\n                      { }
[ \t\r]+                { }
.                       { return ERROR; }

%%

namespace Locic{
	
	namespace Parser{
		
		void * LexAlloc(FILE * file, Context * context){
			void * lexer;
			Locic_Parser_GeneratedLexer_lex_init(&lexer);
			Locic_Parser_GeneratedLexer_set_in(file, lexer);
			Locic_Parser_GeneratedLexer_set_extra(context, lexer);
			return lexer;
		}
		
		int LexGetToken(void * lexer){
			return Locic_Parser_GeneratedLexer_lex(lexer);
		}
		
		void LexFree(void * lexer){
			Locic_Parser_GeneratedLexer_lex_destroy(lexer);
		}
		
	}
	
}


