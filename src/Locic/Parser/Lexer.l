%{

#include <string>
#include <Locic/AST.hpp>
#include <Locic/Parser/Context.hpp>
#include <Locic/Parser/Lexer.hpp>
#include <Locic/Parser/Token.hpp>

#define YY_EXTRA_TYPE Locic::Parser::Context *
#define YY_USER_ACTION yyextra->lineNumber = yylineno;

%}

%option prefix="Locic_Parser_GeneratedLexer_"
%option reentrant
%option noyywrap
%option yylineno

%x COMMENT_STATE
%x STRING_STATE

%%

"/*"                          { BEGIN(COMMENT_STATE); }
<COMMENT_STATE>[^*\n]*       { }
<COMMENT_STATE>"*"+[^*/\n]*  { }
<COMMENT_STATE>\n             { }
<COMMENT_STATE>"*"+"/"        { BEGIN(INITIAL); }

"\""                           { yyextra->stringConstant = ""; BEGIN(STRING_STATE); }
<STRING_STATE>"\\\""          { yyextra->stringConstant += "\""; }
<STRING_STATE>"\\\\"          { yyextra->stringConstant += "\\"; }
<STRING_STATE>"\\n"           { yyextra->stringConstant += "\n"; }
<STRING_STATE>"\\t"           { yyextra->stringConstant += "\t"; }
<STRING_STATE>"\"C"            { yyextra->token.constant = Locic::Constant::String(Locic::Constant::CSTRING, yyextra->stringConstant);
				BEGIN(INITIAL);
				return CONSTANT; }
<STRING_STATE>"\""            { yyextra->token.constant = Locic::Constant::String(Locic::Constant::LOCISTRING, yyextra->stringConstant);
				BEGIN(INITIAL);
				return CONSTANT; }
<STRING_STATE>.               { yyextra->stringConstant += yytext; }

if                      { return IF; }
else                    { return ELSE; }
while                   { return WHILE; }
for                     { return FOR; }
return                  { return RETURN; }
switch                  { return SWITCH; }
case                    { return CASE; }
default                 { return DEFAULT; }
continue                { return CONTINUE; }
break                   { return BREAK; }

in                      { return IN; }
has                     { return HAS; }

exception               { return EXCEPTION; }
throw                   { return THROW; }
try                     { return TRY; }
catch                   { return CATCH; }

template                { return TEMPLATE; }
typename                { return TYPENAME; }
move                    { return MOVE; }

struct                  { return STRUCT; }
enum                    { return ENUM; }
union                   { return UNION; }
class                   { return CLASS; }
interface               { return INTERFACE; }
namespace               { return NAMESPACE; }
datatype                { return DATATYPE; }
__primitive             { return PRIMITIVE; }

auto                    { return AUTO; }
const                   { return CONST; }
constexpr               { return CONSTEXPR; }

using                   { return USING; }
static                  { return STATIC; }
import                  { return IMPORT; }
export                  { return EXPORT; }

new                     { return NEW; }
delete                  { return DELETE; }
extract                 { return EXTRACT; }
sizeof                  { return SIZEOF; }

const_cast              { return CONST_CAST; }
static_cast             { return STATIC_CAST; }
reinterpret_cast        { return REINTERPRET_CAST; }

is_a                    { return IS_A; }
typeid                  { return TYPEID; }
dynamic_cast            { return DYNAMIC_CAST; }

void                    { return VOIDNAME; }

null                    { yyextra->token.constant = Locic::Constant::Null(); return CONSTANT; }
true                    { yyextra->token.constant = Locic::Constant::True(); return CONSTANT; }
false                   { yyextra->token.constant = Locic::Constant::False(); return CONSTANT; }
[0-9]+                  { yyextra->token.constant = Locic::Constant::SignedInt(Locic::Constant::INT, atol(yytext)); return CONSTANT; }
[0-9]+l                 { yyextra->token.constant = Locic::Constant::SignedInt(Locic::Constant::LONG, atol(yytext)); return CONSTANT; }
[0-9]+u                 { yyextra->token.constant = Locic::Constant::UnsignedInt(Locic::Constant::UINT, atol(yytext)); return CONSTANT; }
[0-9]+ul                { yyextra->token.constant = Locic::Constant::UnsignedInt(Locic::Constant::ULONG, atol(yytext)); return CONSTANT; }
[0-9]+\.[0-9]+f         { yyextra->token.constant = Locic::Constant::Float(Locic::Constant::FLOAT, atof(yytext)); return CONSTANT; }
[0-9]+\.[0-9]+          { yyextra->token.constant = Locic::Constant::Float(Locic::Constant::DOUBLE, atof(yytext)); return CONSTANT; }
[0-9]+\.[0-9]+L         { yyextra->token.constant = Locic::Constant::Float(Locic::Constant::LONGDOUBLE, atof(yytext)); return CONSTANT; }

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
"%="                    { return PERCENTEQUAL; }

\/\/(.*?)\n             { }

"%"                     { return PERCENT; }
"="                     { return SETEQUAL; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return STAR; }
"/"                     { return FORWARDSLASH; }
"!"                     { return EXCLAIMMARK; }
"&&"                    { return DOUBLE_AMPERSAND; }
"&"                     { return AMPERSAND; }
"||"                    { return DOUBLE_VERTICAL_BAR; }
"|"                     { return VERTICAL_BAR; }
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
"<<"                    { return DOUBLE_LTRIBRACKET; }
">>"                    { return DOUBLE_RTRIBRACKET; }
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


