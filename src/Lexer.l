%{

#include <string.h>

#include <Locic/AST.h>

#include "parser.tab.h"
int yyerror(char *);

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

%}

%x STRING_STATE
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
auto                    { return AUTO; }

void                    { yylval.name = strcpy(yytext); return VOIDNAME; }
int                     { yylval.name = strcpy(yytext); return INTNAME; }
bool                    { yylval.name = strcpy(yytext); return BOOLNAME; }
float                   { yylval.name = strcpy(yytext); return FLOATNAME; }

true                    { yylval.value = AST_MakeBoolConstant(1); return CONSTANT; }
false                   { yylval.value = AST_MakeBoolConstant(0); return CONSTANT; }
[0-9]+                  { yylval.value = AST_MakeIntConstant(atoi(yytext)); return CONSTANT; }
[0-9]+\.[0-9]+          { yylval.value = AST_MakeFloatConstant(atof(yytext)); return CONSTANT; }

[A-Z][A-Za-z0-9]*       { yylval.name = strcpy(yytext); return UCNAME; }
[a-z][A-Za-z0-9]*       { yylval.name = strcpy(yytext); return LCNAME; }

"=="                    { return ISEQUAL; }
"!="                    { return NOTEQUAL; }
">="                    { return GREATEROREQUAL; }
"<="                    { return LESSOREQUAL; }
"->"                    { return PTRACCESS; }

\/\/(.*?)\n             { }

;                       { return ';'; }
\n                      { }
[ \t\r]+                { }
.                       { return yytext[0]; }

%%


