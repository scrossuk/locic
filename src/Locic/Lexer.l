%{

#include <string.h>
#include <Locic/AST.h>
#include <Locic/Parser.h>
#include <Locic/Token.h>

Locic_Token yylval;

/*
"\""                    { strText = ""; BEGIN(STRING_STATE); }

<STRING_STATE>"\\\""          { strText += "\""; }
<STRING_STATE>"\\\\"          { strText += "\\"; }
<STRING_STATE>"\\n"           { strText += "\n"; }
<STRING_STATE>"\\t"           { strText += "\t"; }
<STRING_STATE>"\""            { yylval.value = new String(strText);
				BEGIN(INITIAL);
				return STRING; }
<STRING_STATE>.               { strText += yytext; }
*/

//%x STRING_STATE

%}

%option noyywrap

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
auto                    { return LOCIC_TOKEN_AUTO; }
const                   { return LOCIC_TOKEN_CONST; }

void                    { yylval.str = strcpy(malloc(strlen(yytext) + 1), yytext); return LOCIC_TOKEN_VOIDNAME; }
int                     { yylval.str = strcpy(malloc(strlen(yytext) + 1), yytext); return LOCIC_TOKEN_INTNAME; }
bool                    { yylval.str = strcpy(malloc(strlen(yytext) + 1), yytext); return LOCIC_TOKEN_BOOLNAME; }
float                   { yylval.str = strcpy(malloc(strlen(yytext) + 1), yytext); return LOCIC_TOKEN_FLOATNAME; }

true                    { yylval.boolValue = 1; return LOCIC_TOKEN_BOOLCONSTANT; }
false                   { yylval.boolValue = 0; return LOCIC_TOKEN_BOOLCONSTANT; }
[0-9]+                  { yylval.intValue = atoi(yytext); return LOCIC_TOKEN_INTCONSTANT; }
[0-9]+\.[0-9]+          { yylval.floatValue = atof(yytext); return LOCIC_TOKEN_FLOATCONSTANT; }

[A-Z][A-Za-z0-9]*       { yylval.str = strcpy(malloc(strlen(yytext) + 1), yytext); return LOCIC_TOKEN_UCNAME; }
[a-z][A-Za-z0-9]*       { yylval.str = strcpy(malloc(strlen(yytext) + 1), yytext); return LOCIC_TOKEN_LCNAME; }

"=="                    { return LOCIC_TOKEN_ISEQUAL; }
"!="                    { return LOCIC_TOKEN_NOTEQUAL; }
">="                    { return LOCIC_TOKEN_GREATEROREQUAL; }
"<="                    { return LOCIC_TOKEN_LESSOREQUAL; }
"->"                    { return LOCIC_TOKEN_PTRACCESS; }

\/\/(.*?)\n             { }

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
\n                      { }
[ \t\r]+                { }
.                       { return LOCIC_TOKEN_ERROR; }

%%


