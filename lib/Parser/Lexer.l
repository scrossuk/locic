%{

#include <string>

#include <locic/AST.hpp>
#include <locic/Version.hpp>
#include <locic/Parser/Context.hpp>

#include "Lexer.hpp"
#include "LocationInfo.hpp"
#include "Token.hpp"

#define YY_EXTRA_TYPE locic::Parser::Context *
#define YYSTYPE locic::Parser::Token
#define YYLTYPE locic::Parser::LocationInfo
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
	yylloc->first_column = yyextra->columnPosition(); yylloc->last_column = yyextra->columnPosition() + yyleng; \
	yyextra->advanceColumn(yyleng);

// To fix 'unused parameter' warnings.
#define YY_EXIT_FAILURE ((void) yyscanner, 2)

%}

%option prefix="Locic_Parser_GeneratedLexer_"
%option reentrant

%option noyyalloc
%option noyyrealloc
%option noyyfree
%option noyywrap
%option noinput
%option nounput

%option yylineno

%option bison-bridge bison-locations

%x COMMENT_STATE
%x STRING_STATE

%%

"/*"                          { BEGIN(COMMENT_STATE); }
<COMMENT_STATE>[^*\n]*        { }
<COMMENT_STATE>"*"+[^*/\n]*   { }
<COMMENT_STATE>\n             { yyextra->resetColumn(); }
<COMMENT_STATE>"*"+"/"        { BEGIN(INITIAL); }

"\""                          { yyextra->resetStringConstant(); BEGIN(STRING_STATE); }
<STRING_STATE>"\\\""          { yyextra->appendStringConstant("\""); }
<STRING_STATE>"\\\\"          { yyextra->appendStringConstant("\\"); }
<STRING_STATE>"\\n"           { yyextra->appendStringConstant("\n"); }
<STRING_STATE>"\\t"           { yyextra->appendStringConstant("\t"); }
<STRING_STATE>"\""            { yylval->lexer_constant = locic::Constant::String(yyextra->getStringConstant());
				BEGIN(INITIAL);
				return CONSTANT; }
<STRING_STATE>.               { yyextra->appendStringConstant(yytext); }

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

exception               { return EXCEPTION; }
throw                   { return THROW; }
try                     { return TRY; }
catch                   { return CATCH; }
scope                   { return SCOPE; }
noexcept                { return NOEXCEPT; }

template                { return TEMPLATE; }
typename                { return TYPENAME; }
virtual                 { return VIRTUAL; }

self                    { return SELF; }
this                    { return THIS; }

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
final                   { return FINAL; }
lval                    { return LVAL; }
nolval                  { return NOLVAL; }
ref                     { return REF; }
noref                   { return NOREF; }

using                   { return USING; }
static                  { return STATIC; }
import                  { return IMPORT; }
export                  { return EXPORT; }

move                    { return MOVE; }
sizeof                  { return SIZEOF; }

cast                    { return CAST; }
const_cast              { return CONST_CAST; }
static_cast             { return STATIC_CAST; }
reinterpret_cast        { return REINTERPRET_CAST; }
dynamic_cast            { return DYNAMIC_CAST; }

typeof                  { return TYPEOF; }
typeid                  { return TYPEID; }

void                    { return VOID; }
signed                  { return SIGNED; }
unsigned                { return UNSIGNED; }
byte                    { return BYTE; }
short                   { return SHORT; }
int                     { return INT; }
long                    { return LONG; }
float                   { return FLOAT; }
double                  { return DOUBLE; }

null                    { return NULLVAL; }
true                    { yylval->lexer_constant = locic::Constant::True(); return CONSTANT; }
false                   { yylval->lexer_constant = locic::Constant::False(); return CONSTANT; }
[0-9]+                  { yylval->lexer_constant = locic::Constant::Integer(atol(yytext)); return CONSTANT; }
[0-9]+\.[0-9]+          { yylval->lexer_constant = locic::Constant::Float(atof(yytext)); return CONSTANT; }

[0-9]+\.[0-9]+.[0-9]+   { yylval->lexer_version = new locic::Version(locic::Version::FromString(yytext)); return VERSION; }

"_"                     { return UNDERSCORE; }

[A-Za-z_][A-Za-z0-9_]*  { yylval->lexer_str = new std::string(yytext); return NAME; }

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
"["                     { return LSQUAREBRACKET; }
"]"                     { return RSQUAREBRACKET; }
"."                     { return DOT; }
"<<"                    { return DOUBLE_LTRIBRACKET; }
"<"                     { return LTRIBRACKET; }
">"                     { return RTRIBRACKET; }
"~"                     { return TILDA; }
\n                      { yyextra->resetColumn(); }
[ \t\r]+                { }
.                       { return ERROR; }

%%

void * Locic_Parser_GeneratedLexer_alloc(yy_size_t size, yyscan_t yyscanner) {
	(void) yyscanner;
	return malloc(size);
}

void * Locic_Parser_GeneratedLexer_realloc(void * ptr, yy_size_t size, yyscan_t yyscanner) {
	(void) yyscanner;
	return realloc(ptr, size);
}

void Locic_Parser_GeneratedLexer_free(void * ptr, yyscan_t yyscanner) {
	(void) yyscanner;
	free(ptr);
}

namespace locic {
	
	namespace Parser {
		
		void * LexAlloc(FILE * file, Context * context){
			void * lexer;
			Locic_Parser_GeneratedLexer_lex_init(&lexer);
			Locic_Parser_GeneratedLexer_set_in(file, lexer);
			Locic_Parser_GeneratedLexer_set_extra(context, lexer);
			return lexer;
		}
		
		int LexGetToken(Token* token, LocationInfo* locationInfo, void * lexer){
			return Locic_Parser_GeneratedLexer_lex(token, locationInfo, lexer);
		}
		
		void LexFree(void * lexer){
			Locic_Parser_GeneratedLexer_lex_destroy(lexer);
		}
		
	}
	
}


