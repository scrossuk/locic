%{

#include <string>

#include "Context.h"
#include "Tree.h"
#include "Class.h"
#include "ParamSpec.h"
#include "StringTree.h"
#include "BoolTree.h"
#include "UintTree.h"
#include "FloatTree.h"

#include "Parser.tab.h"
int yyerror(char *);

static std::string strText;

extern Locic::Context * globalContext;

%}

%option noyywrap

%x STRING_STATE
%x COMMENT_STATE

%%

"\""                    { strText = ""; BEGIN(STRING_STATE); }

<STRING_STATE>"\\\""          { strText += "\""; }
<STRING_STATE>"\\\\"          { strText += "\\"; }
<STRING_STATE>"\\n"           { strText += "\n"; }
<STRING_STATE>"\\t"           { strText += "\t"; }
<STRING_STATE>"\""            { yylval.tree = new Locic::StringTree(globalContext->Resolve("String"), strText);
								BEGIN(INITIAL);
								return STRING; }
<STRING_STATE>.               { strText += yytext; }

"/*"                    { BEGIN(COMMENT_STATE); }
<COMMENT_STATE>[^*\n]*        { }
<COMMENT_STATE>"*"+[^*/\n]*   { }
<COMMENT_STATE>\n             { }
<COMMENT_STATE>"*"+"/"        { BEGIN(INITIAL); }

if                      { return IF; }
else                    { return ELSE; }
external                { return EXTERNAL; }

true                    { yylval.tree = new Locic::BoolTree(globalContext->Resolve("Bool"), true); return BOOL; }
false                   { yylval.tree = new Locic::BoolTree(globalContext->Resolve("Bool"), false); return BOOL; }

[0-9]+                  { yylval.tree = new Locic::UintTree(globalContext->Resolve("Uint"), atoi(yytext)); return INT; }
[0-9]+\.[0-9]+          { yylval.tree = new Locic::FloatTree(globalContext->Resolve("Float"), atof(yytext)); return FLOAT; }

[A-Z][A-Za-z0-9]*       { yylval.string = new std::string(yytext); return TYPENAME; }
[a-z][A-Za-z0-9]*       { yylval.string = new std::string(yytext); return VARNAME; }

"=="                    { return ISEQUAL; }
"!="                    { return NOTEQUAL; }
">="                    { return GREATEROREQUAL; }
"<="                    { return LESSOREQUAL; }
"->"                    { return PTRACCESS; }

\/\/(.*?)\n             { }

;                       { return ';'; }
\n                      { return ';'; }
[ \t\r]+                { }
.                       { return yytext[0]; }

%%


