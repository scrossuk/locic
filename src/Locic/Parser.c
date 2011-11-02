/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 1 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
#include <assert.h>
#line 2 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
#include <stdio.h>
#line 3 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
#include <Locic/Token.h>
#line 4 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
#include <Locic/AST.h>
#line 16 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    Locic_ParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is Locic_ParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    Locic_ParseARG_SDECL     A static variable declaration for the %extra_argument
**    Locic_ParseARG_PDECL     A parameter declaration for the %extra_argument
**    Locic_ParseARG_STORE     Code to store %extra_argument into yypParser
**    Locic_ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 75
#define YYACTIONTYPE unsigned short int
#define Locic_ParseTOKENTYPE  Locic_Token 
typedef union {
  int yyinit;
  Locic_ParseTOKENTYPE yy0;
  char * yy45;
  AST_ClassDecl * yy52;
  AST_ClassDef * yy59;
  AST_ClassMethodDecl * yy65;
  AST_ClassMethodDef * yy68;
  AST_Value * yy75;
  AST_Scope * yy76;
  AST_Statement * yy99;
  AST_Type * yy100;
  AST_List * yy102;
  AST_TypeVar * yy121;
  AST_File * yy122;
  int yy149;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define Locic_ParseARG_SDECL  AST_File ** resultAST ;
#define Locic_ParseARG_PDECL , AST_File ** resultAST 
#define Locic_ParseARG_FETCH  AST_File ** resultAST  = yypParser->resultAST 
#define Locic_ParseARG_STORE yypParser->resultAST  = resultAST 
#define YYNSTATE 183
#define YYNRULE 86
#define YYERRORSYMBOL 42
#define YYERRSYMDT yy149
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
#define YY_ACTTAB_COUNT (478)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   132,   15,   28,  179,  171,  177,  176,  175,  174,   41,
 /*    10 */    31,    5,  169,   91,   61,   88,   26,   85,   47,  114,
 /*    20 */    50,    6,  155,  154,  153,  167,   55,   35,   34,   33,
 /*    30 */    32,   43,   72,   81,   40,   39,  173,   38,   71,  157,
 /*    40 */    77,  101,  149,  100,   27,  131,  137,  138,   98,   62,
 /*    50 */    69,  142,   70,  150,   71,  157,   68,  142,   70,  150,
 /*    60 */    71,  157,  152,  105,   94,  119,  270,   46,  137,  138,
 /*    70 */    98,   62,   69,  142,   70,  150,   71,  157,   67,  142,
 /*    80 */    70,  150,   45,  178,  152,  105,  102,  173,  166,  110,
 /*    90 */   137,  138,   98,   62,   69,  142,   70,  150,   71,  157,
 /*   100 */   179,  163,  177,  176,  175,  174,  152,  105,   99,  101,
 /*   110 */   136,   96,  137,  138,   98,   62,   69,  142,   70,  150,
 /*   120 */   108,   71,  157,   30,   15,  183,  179,  171,   58,  152,
 /*   130 */   105,   95,  164,   31,  112,  137,  138,   98,   62,   69,
 /*   140 */   142,   70,  150,   56,   29,  155,  154,  153,   71,  157,
 /*   150 */    35,   34,   33,   32,   60,  179,  159,  177,  176,  175,
 /*   160 */   174,   41,  137,  138,   98,   62,   69,  142,   70,  150,
 /*   170 */    25,   71,  157,  170,   87,  180,   23,   22,  179,   92,
 /*   180 */   177,  176,  175,  174,   41,  137,  138,   98,   62,   69,
 /*   190 */   142,   70,  150,   71,  157,  170,   83,  161,   71,  157,
 /*   200 */   179,   90,  177,  176,  175,  174,   41,  137,  138,   98,
 /*   210 */    62,   69,  142,   70,  150,   71,  157,  147,   70,  150,
 /*   220 */   182,  181,  171,   86,   71,  157,  170,  168,  116,  137,
 /*   230 */   138,   98,   62,   69,  142,   70,  150,   14,   71,  157,
 /*   240 */    57,   66,   69,  142,   70,  150,   84,  170,   93,  164,
 /*   250 */   158,   75,  137,  138,   98,   62,   69,  142,   70,  150,
 /*   260 */    71,  157,  120,  118,   23,   22,   71,  157,   82,   21,
 /*   270 */    20,   19,   18,    4,  137,  138,   98,   62,   69,  142,
 /*   280 */    70,  150,   71,  157,  151,  146,   70,  150,  148,  178,
 /*   290 */    80,   71,  157,  172,  195,   17,  137,  138,   98,   62,
 /*   300 */    69,  142,   70,  150,    3,   71,  157,   16,   65,   69,
 /*   310 */   142,   70,  150,   78,    2,   13,  135,  134,  133,  137,
 /*   320 */   138,   98,   62,   69,  142,   70,  150,   71,  157,   12,
 /*   330 */   162,   54,   53,   44,  117,   76,   71,  157,  173,   48,
 /*   340 */    11,  137,  138,   98,   62,   69,  142,   70,  150,   10,
 /*   350 */   104,  138,   98,   62,   69,  142,   70,  150,   71,  157,
 /*   360 */    52,   51,    9,   45,  178,  127,   71,  157,  173,  165,
 /*   370 */   113,  115,    8,   97,   98,   62,   69,  142,   70,  150,
 /*   380 */   126,  139,   98,   62,   69,  142,   70,  150,   71,  157,
 /*   390 */     7,   24,  125,  124,   71,  157,   45,  178,  123,   71,
 /*   400 */   157,  173,  165,  113,  111,   64,   69,  142,   70,  150,
 /*   410 */    49,   63,   69,  142,   70,  150,   45,  178,  145,   70,
 /*   420 */   150,  173,  165,  113,  109,   45,  178,   71,  157,   36,
 /*   430 */   173,  165,  113,  106,   45,  178,   71,  157,   37,  173,
 /*   440 */   165,  113,   74,    1,   73,  179,  144,   70,  150,   71,
 /*   450 */   157,  160,   71,  157,   59,  143,   70,  150,  121,   42,
 /*   460 */   107,  171,  156,  103,  173,   89,  271,  130,  141,   70,
 /*   470 */   150,  140,   70,  150,   79,  129,  128,  122,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     5,    6,    6,    8,    9,   10,   11,   12,   13,   14,
 /*    10 */    15,    6,   14,   18,    4,   20,    6,   22,   23,    7,
 /*    20 */    25,   26,   27,   28,   29,   16,   21,   32,   33,   34,
 /*    30 */    35,   51,   52,   53,   30,   31,   56,   17,   52,   53,
 /*    40 */    60,   53,   54,   55,    6,   65,   66,   67,   68,   69,
 /*    50 */    70,   71,   72,   73,   52,   53,   70,   71,   72,   73,
 /*    60 */    52,   53,   60,   61,   62,   42,   43,   44,   66,   67,
 /*    70 */    68,   69,   70,   71,   72,   73,   52,   53,   70,   71,
 /*    80 */    72,   73,   51,   52,   60,   61,   62,   56,   57,    7,
 /*    90 */    66,   67,   68,   69,   70,   71,   72,   73,   52,   53,
 /*   100 */     8,   16,   10,   11,   12,   13,   60,   61,   62,   53,
 /*   110 */    54,   55,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   120 */     7,   52,   53,   15,    6,    0,    8,    9,    3,   60,
 /*   130 */    61,   62,   53,   15,   55,   66,   67,   68,   69,   70,
 /*   140 */    71,   72,   73,   25,   36,   27,   28,   29,   52,   53,
 /*   150 */    32,   33,   34,   35,    4,    8,   60,   10,   11,   12,
 /*   160 */    13,   14,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   170 */     6,   52,   53,   53,   54,    5,   32,   33,    8,   60,
 /*   180 */    10,   11,   12,   13,   14,   66,   67,   68,   69,   70,
 /*   190 */    71,   72,   73,   52,   53,   53,   54,    5,   52,   53,
 /*   200 */     8,   60,   10,   11,   12,   13,   14,   66,   67,   68,
 /*   210 */    69,   70,   71,   72,   73,   52,   53,   71,   72,   73,
 /*   220 */    45,   46,    9,   60,   52,   53,   53,   54,   15,   66,
 /*   230 */    67,   68,   69,   70,   71,   72,   73,   17,   52,   53,
 /*   240 */     7,   69,   70,   71,   72,   73,   60,   53,   54,   53,
 /*   250 */     7,   55,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   260 */    52,   53,    1,    2,   32,   33,   52,   53,   60,   37,
 /*   270 */    38,   39,   40,    6,   66,   67,   68,   69,   70,   71,
 /*   280 */    72,   73,   52,   53,    7,   71,   72,   73,    7,   52,
 /*   290 */    60,   52,   53,   56,    6,   41,   66,   67,   68,   69,
 /*   300 */    70,   71,   72,   73,    6,   52,   53,   21,   69,   70,
 /*   310 */    71,   72,   73,   60,    6,   24,    7,    7,   16,   66,
 /*   320 */    67,   68,   69,   70,   71,   72,   73,   52,   53,    6,
 /*   330 */    48,    7,   19,   51,   52,   60,   52,   53,   56,    6,
 /*   340 */    21,   66,   67,   68,   69,   70,   71,   72,   73,    6,
 /*   350 */    66,   67,   68,   69,   70,   71,   72,   73,   52,   53,
 /*   360 */     7,    7,   24,   51,   52,   16,   52,   53,   56,   57,
 /*   370 */    58,   59,   24,   67,   68,   69,   70,   71,   72,   73,
 /*   380 */    16,   67,   68,   69,   70,   71,   72,   73,   52,   53,
 /*   390 */    24,    6,   16,   16,   52,   53,   51,   52,   16,   52,
 /*   400 */    53,   56,   57,   58,   59,   69,   70,   71,   72,   73,
 /*   410 */     7,   69,   70,   71,   72,   73,   51,   52,   71,   72,
 /*   420 */    73,   56,   57,   58,   59,   51,   52,   52,   53,   49,
 /*   430 */    56,   57,   58,   59,   51,   52,   52,   53,   47,   56,
 /*   440 */    57,   58,   59,   64,   52,    8,   71,   72,   73,   52,
 /*   450 */    53,   63,   52,   53,    4,   71,   72,   73,   50,   51,
 /*   460 */    52,    9,   53,   52,   56,   63,   74,   63,   71,   72,
 /*   470 */    73,   71,   72,   73,   53,   63,   63,   63,
};
#define YY_SHIFT_USE_DFLT (-6)
#define YY_SHIFT_COUNT (117)
#define YY_SHIFT_MIN   (-5)
#define YY_SHIFT_MAX   (452)
static const short yy_shift_ofst[] = {
 /*     0 */   261,   -5,  118,  118,  118,  118,  118,  118,  118,  118,
 /*    10 */   118,  118,  118,  118,  118,  118,  118,  118,  118,  118,
 /*    20 */   118,  118,  118,  118,  147,  147,  147,  147,  147,  118,
 /*    30 */   118,  118,  118,  118,  118,  118,  192,  170,  147,  452,
 /*    40 */   452,   92,  213,  213,  213,  213,  125,  452,  452,  450,
 /*    50 */   452,  450,  450,  450,  450,  437,  452,  450,  437,   -6,
 /*    60 */    -6,   -6,  232,  144,  144,  144,  144,  108,  108,  108,
 /*    70 */     4,    5,    5,   10,  403,  385,  382,  377,  376,  366,
 /*    80 */   364,  348,  349,  338,  354,  343,  353,  319,  333,  313,
 /*    90 */   324,  323,  302,  291,  310,  309,  308,  286,  254,  281,
 /*   100 */   298,  288,  277,  267,  243,  220,  233,  164,  150,  113,
 /*   110 */    85,   82,   38,   20,    9,   12,   -2,   -4,
};
#define YY_REDUCE_USE_DFLT (-21)
#define YY_REDUCE_COUNT (61)
#define YY_REDUCE_MIN   (-20)
#define YY_REDUCE_MAX   (421)
static const short yy_reduce_ofst[] = {
 /*     0 */    23,  -20,   69,   46,   24,    2,  275,  253,  230,  208,
 /*    10 */   186,  163,  141,  119,   96,  284,  314,  306,  342,  336,
 /*    20 */   239,  172,    8,  -14,  383,  374,  365,  345,  312,  400,
 /*    30 */   397,  384,  375,  347,  214,  146,  408,  282,   31,   56,
 /*    40 */   -12,  237,  196,  194,   79,  173,  175,  142,  120,  414,
 /*    50 */   421,  413,  412,  404,  402,  411,  409,  388,  392,  379,
 /*    60 */   380,  391,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   184,  269,  218,  218,  218,  218,  269,  269,  269,  269,
 /*    10 */   269,  269,  269,  269,  269,  269,  269,  269,  269,  269,
 /*    20 */   269,  269,  269,  269,  214,  214,  214,  214,  214,  269,
 /*    30 */   269,  269,  269,  269,  269,  269,  269,  269,  269,  269,
 /*    40 */   269,  269,  269,  269,  269,  269,  269,  269,  269,  269,
 /*    50 */   269,  269,  269,  269,  269,  269,  269,  269,  269,  223,
 /*    60 */   207,  205,  260,  264,  263,  262,  261,  259,  258,  257,
 /*    70 */   248,  269,  196,  269,  269,  269,  269,  269,  269,  237,
 /*    80 */   269,  236,  269,  269,  269,  269,  269,  269,  269,  225,
 /*    90 */   269,  269,  269,  269,  269,  269,  269,  269,  265,  269,
 /*   100 */   269,  194,  269,  269,  269,  219,  269,  196,  269,  269,
 /*   110 */   269,  269,  269,  215,  269,  269,  203,  196,  187,  186,
 /*   120 */   185,  208,  212,  234,  233,  232,  231,  229,  228,  227,
 /*   130 */   226,  224,  222,  230,  241,  247,  246,  268,  267,  266,
 /*   140 */   256,  255,  254,  253,  252,  251,  250,  249,  245,  244,
 /*   150 */   243,  242,  220,  240,  239,  238,  237,  236,  235,  221,
 /*   160 */   211,  191,  206,  210,  195,  216,  217,  209,  213,  204,
 /*   170 */   194,  193,  202,  201,  200,  199,  198,  197,  196,  192,
 /*   180 */   190,  189,  188,
};

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  Locic_ParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void Locic_ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "ERROR",         "INTERFACE",     "CLASS",       
  "LCURLYBRACKET",  "RCURLYBRACKET",  "LROUNDBRACKET",  "RROUNDBRACKET",
  "UCNAME",        "LCNAME",        "VOIDNAME",      "BOOLNAME",    
  "INTNAME",       "FLOATNAME",     "CONST",         "STAR",        
  "SEMICOLON",     "COMMA",         "IF",            "ELSE",        
  "FOR",           "COLON",         "WHILE",         "AUTO",        
  "SETEQUAL",      "AT",            "RETURN",        "BOOLCONSTANT",
  "INTCONSTANT",   "FLOATCONSTANT",  "DOT",           "PTRACCESS",   
  "PLUS",          "MINUS",         "EXCLAIMMARK",   "AMPERSAND",   
  "FORWARDSLASH",  "ISEQUAL",       "NOTEQUAL",      "GREATEROREQUAL",
  "LESSOREQUAL",   "QUESTIONMARK",  "error",         "start",       
  "file",          "classDecl",     "classDef",      "classMethodDeclList",
  "classMethodDecl",  "classMethodDefList",  "classMethodDef",  "type",        
  "ucName",        "lcName",        "varName",       "methodName",  
  "typeName",      "typeVar",       "nonEmptyTypeVarList",  "typeVarList", 
  "value",         "nonEmptyValueList",  "valueList",     "scope",       
  "statementList",  "statement",     "precision0",    "precision1",  
  "precision2",    "precision3",    "precision4",    "precision5",  
  "precision6",    "precision7",  
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "start ::= file",
 /*   1 */ "file ::=",
 /*   2 */ "file ::= ERROR",
 /*   3 */ "file ::= error",
 /*   4 */ "file ::= INTERFACE",
 /*   5 */ "file ::= file classDecl",
 /*   6 */ "file ::= file classDef",
 /*   7 */ "classDecl ::= CLASS ucName LCURLYBRACKET classMethodDeclList RCURLYBRACKET",
 /*   8 */ "classDef ::= CLASS ucName LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classMethodDefList RCURLYBRACKET",
 /*   9 */ "ucName ::= UCNAME",
 /*  10 */ "lcName ::= LCNAME",
 /*  11 */ "varName ::= lcName",
 /*  12 */ "methodName ::= lcName",
 /*  13 */ "typeName ::= ucName",
 /*  14 */ "typeName ::= VOIDNAME",
 /*  15 */ "typeName ::= BOOLNAME",
 /*  16 */ "typeName ::= INTNAME",
 /*  17 */ "typeName ::= FLOATNAME",
 /*  18 */ "type ::= typeName",
 /*  19 */ "type ::= CONST typeName",
 /*  20 */ "type ::= type STAR",
 /*  21 */ "type ::= type STAR CONST",
 /*  22 */ "classMethodDeclList ::=",
 /*  23 */ "classMethodDeclList ::= classMethodDeclList classMethodDecl",
 /*  24 */ "classMethodDefList ::=",
 /*  25 */ "classMethodDefList ::= classMethodDefList classMethodDef",
 /*  26 */ "classMethodDecl ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON",
 /*  27 */ "classMethodDecl ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON",
 /*  28 */ "classMethodDef ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET scope",
 /*  29 */ "classMethodDef ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET scope",
 /*  30 */ "typeVar ::= type varName",
 /*  31 */ "typeVarList ::=",
 /*  32 */ "typeVarList ::= nonEmptyTypeVarList",
 /*  33 */ "nonEmptyTypeVarList ::= typeVar",
 /*  34 */ "nonEmptyTypeVarList ::= nonEmptyTypeVarList COMMA typeVar",
 /*  35 */ "valueList ::=",
 /*  36 */ "valueList ::= nonEmptyValueList",
 /*  37 */ "nonEmptyValueList ::= value",
 /*  38 */ "nonEmptyValueList ::= nonEmptyValueList COMMA value",
 /*  39 */ "scope ::= LCURLYBRACKET statementList RCURLYBRACKET",
 /*  40 */ "statementList ::=",
 /*  41 */ "statementList ::= statementList statement",
 /*  42 */ "statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope",
 /*  43 */ "statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope",
 /*  44 */ "statement ::= FOR LROUNDBRACKET varName COLON value RROUNDBRACKET scope",
 /*  45 */ "statement ::= WHILE LROUNDBRACKET value RROUNDBRACKET scope",
 /*  46 */ "statement ::= AUTO varName SETEQUAL value SEMICOLON",
 /*  47 */ "statement ::= type varName SETEQUAL value SEMICOLON",
 /*  48 */ "statement ::= lcName SETEQUAL value SEMICOLON",
 /*  49 */ "statement ::= AT lcName SETEQUAL value SEMICOLON",
 /*  50 */ "statement ::= value SEMICOLON",
 /*  51 */ "statement ::= RETURN value SEMICOLON",
 /*  52 */ "precision7 ::= LROUNDBRACKET precision0 RROUNDBRACKET",
 /*  53 */ "precision7 ::= lcName",
 /*  54 */ "precision7 ::= AT lcName",
 /*  55 */ "precision7 ::= BOOLCONSTANT",
 /*  56 */ "precision7 ::= INTCONSTANT",
 /*  57 */ "precision7 ::= FLOATCONSTANT",
 /*  58 */ "precision7 ::= ucName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  59 */ "precision7 ::= ucName COLON ucName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  60 */ "precision6 ::= precision7",
 /*  61 */ "precision6 ::= precision6 DOT varName",
 /*  62 */ "precision6 ::= precision6 DOT methodName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  63 */ "precision6 ::= precision6 PTRACCESS varName",
 /*  64 */ "precision6 ::= precision6 PTRACCESS methodName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  65 */ "precision5 ::= precision6",
 /*  66 */ "precision5 ::= PLUS precision5",
 /*  67 */ "precision5 ::= MINUS precision5",
 /*  68 */ "precision5 ::= EXCLAIMMARK precision5",
 /*  69 */ "precision5 ::= AMPERSAND precision5",
 /*  70 */ "precision5 ::= STAR precision5",
 /*  71 */ "precision4 ::= precision5",
 /*  72 */ "precision4 ::= precision4 STAR precision5",
 /*  73 */ "precision4 ::= precision4 FORWARDSLASH precision5",
 /*  74 */ "precision3 ::= precision4",
 /*  75 */ "precision3 ::= precision3 PLUS precision4",
 /*  76 */ "precision3 ::= precision3 MINUS precision4",
 /*  77 */ "precision2 ::= precision3",
 /*  78 */ "precision2 ::= precision3 ISEQUAL precision3",
 /*  79 */ "precision2 ::= precision3 NOTEQUAL precision3",
 /*  80 */ "precision2 ::= precision3 GREATEROREQUAL precision3",
 /*  81 */ "precision2 ::= precision3 LESSOREQUAL precision3",
 /*  82 */ "precision1 ::= precision2",
 /*  83 */ "precision1 ::= precision2 QUESTIONMARK precision1 COLON precision1",
 /*  84 */ "precision0 ::= precision1",
 /*  85 */ "value ::= precision0",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Locic_Parse and Locic_ParseFree.
*/
void *Locic_ParseAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  Locic_ParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from Locic_ParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void Locic_ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int Locic_ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_COUNT
   || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   Locic_ParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
   Locic_ParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 43, 1 },
  { 44, 0 },
  { 44, 1 },
  { 44, 1 },
  { 44, 1 },
  { 44, 2 },
  { 44, 2 },
  { 45, 5 },
  { 46, 8 },
  { 52, 1 },
  { 53, 1 },
  { 54, 1 },
  { 55, 1 },
  { 56, 1 },
  { 56, 1 },
  { 56, 1 },
  { 56, 1 },
  { 56, 1 },
  { 51, 1 },
  { 51, 2 },
  { 51, 2 },
  { 51, 3 },
  { 47, 0 },
  { 47, 2 },
  { 49, 0 },
  { 49, 2 },
  { 48, 5 },
  { 48, 6 },
  { 50, 5 },
  { 50, 6 },
  { 57, 2 },
  { 59, 0 },
  { 59, 1 },
  { 58, 1 },
  { 58, 3 },
  { 62, 0 },
  { 62, 1 },
  { 61, 1 },
  { 61, 3 },
  { 63, 3 },
  { 64, 0 },
  { 64, 2 },
  { 65, 5 },
  { 65, 7 },
  { 65, 7 },
  { 65, 5 },
  { 65, 5 },
  { 65, 5 },
  { 65, 4 },
  { 65, 5 },
  { 65, 2 },
  { 65, 3 },
  { 73, 3 },
  { 73, 1 },
  { 73, 2 },
  { 73, 1 },
  { 73, 1 },
  { 73, 1 },
  { 73, 4 },
  { 73, 6 },
  { 72, 1 },
  { 72, 3 },
  { 72, 6 },
  { 72, 3 },
  { 72, 6 },
  { 71, 1 },
  { 71, 2 },
  { 71, 2 },
  { 71, 2 },
  { 71, 2 },
  { 71, 2 },
  { 70, 1 },
  { 70, 3 },
  { 70, 3 },
  { 69, 1 },
  { 69, 3 },
  { 69, 3 },
  { 68, 1 },
  { 68, 3 },
  { 68, 3 },
  { 68, 3 },
  { 68, 3 },
  { 67, 1 },
  { 67, 5 },
  { 66, 1 },
  { 60, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  Locic_ParseARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* start ::= file */
#line 66 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		printf("Completed parsing\n");
		yygotominor.yy122 = yymsp[0].minor.yy122;
	}
#line 981 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 1: /* file ::= */
      case 2: /* file ::= ERROR */ yytestcase(yyruleno==2);
      case 4: /* file ::= INTERFACE */ yytestcase(yyruleno==4);
#line 72 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy122 = AST_MakeFile();
	}
#line 990 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 3: /* file ::= error */
#line 82 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		printf("ERROR\n");
		yygotominor.yy122 = AST_MakeFile();
	}
#line 998 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 5: /* file ::= file classDecl */
#line 93 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy122 = AST_FileAddClassDecl(yymsp[-1].minor.yy122, yymsp[0].minor.yy52);
	}
#line 1005 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 6: /* file ::= file classDef */
#line 98 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy122 = AST_FileAddClassDef(yymsp[-1].minor.yy122, yymsp[0].minor.yy59);
	}
#line 1012 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 7: /* classDecl ::= CLASS ucName LCURLYBRACKET classMethodDeclList RCURLYBRACKET */
#line 103 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy52 = AST_MakeClassDecl(yymsp[-3].minor.yy45, yymsp[-1].minor.yy102);
	}
#line 1019 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 8: /* classDef ::= CLASS ucName LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classMethodDefList RCURLYBRACKET */
#line 108 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy59 = AST_MakeClassDef(yymsp[-6].minor.yy45, yymsp[-4].minor.yy102, yymsp[-1].minor.yy102);
	}
#line 1026 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 9: /* ucName ::= UCNAME */
      case 10: /* lcName ::= LCNAME */ yytestcase(yyruleno==10);
      case 14: /* typeName ::= VOIDNAME */ yytestcase(yyruleno==14);
      case 15: /* typeName ::= BOOLNAME */ yytestcase(yyruleno==15);
      case 16: /* typeName ::= INTNAME */ yytestcase(yyruleno==16);
      case 17: /* typeName ::= FLOATNAME */ yytestcase(yyruleno==17);
#line 113 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy45 = (yymsp[0].minor.yy0).value.str;
	}
#line 1038 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 11: /* varName ::= lcName */
      case 12: /* methodName ::= lcName */ yytestcase(yyruleno==12);
      case 13: /* typeName ::= ucName */ yytestcase(yyruleno==13);
#line 123 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy45 = yymsp[0].minor.yy45;
	}
#line 1047 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 18: /* type ::= typeName */
#line 158 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy100 = AST_MakeNamedType(AST_TYPE_MUTABLE, yymsp[0].minor.yy45);
	}
#line 1054 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 19: /* type ::= CONST typeName */
#line 163 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy100 = AST_MakeNamedType(AST_TYPE_CONST, yymsp[0].minor.yy45);
	}
#line 1061 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 20: /* type ::= type STAR */
#line 168 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy100 = AST_MakePtrType(AST_TYPE_MUTABLE, yymsp[-1].minor.yy100);
	}
#line 1068 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 21: /* type ::= type STAR CONST */
#line 173 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy100 = AST_MakePtrType(AST_TYPE_CONST, yymsp[-2].minor.yy100);
	}
#line 1075 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 22: /* classMethodDeclList ::= */
      case 24: /* classMethodDefList ::= */ yytestcase(yyruleno==24);
      case 31: /* typeVarList ::= */ yytestcase(yyruleno==31);
      case 35: /* valueList ::= */ yytestcase(yyruleno==35);
      case 40: /* statementList ::= */ yytestcase(yyruleno==40);
#line 178 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = AST_ListCreate();
	}
#line 1086 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 23: /* classMethodDeclList ::= classMethodDeclList classMethodDecl */
#line 183 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = AST_ListAppend(yymsp[-1].minor.yy102, yymsp[0].minor.yy65);
	}
#line 1093 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 25: /* classMethodDefList ::= classMethodDefList classMethodDef */
#line 193 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = AST_ListAppend(yymsp[-1].minor.yy102, yymsp[0].minor.yy68);
	}
#line 1100 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 26: /* classMethodDecl ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON */
#line 198 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy65 = AST_MakeClassMethodDecl(0, yymsp[-4].minor.yy45, yymsp[-2].minor.yy102);
	}
#line 1107 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 27: /* classMethodDecl ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON */
#line 203 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy65 = AST_MakeClassMethodDecl(yymsp[-5].minor.yy100, yymsp[-4].minor.yy45, yymsp[-2].minor.yy102);
	}
#line 1114 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 28: /* classMethodDef ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET scope */
#line 208 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy68 = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(0, yymsp[-4].minor.yy45, yymsp[-2].minor.yy102), yymsp[0].minor.yy76);
	}
#line 1121 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 29: /* classMethodDef ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET scope */
#line 213 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy68 = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(yymsp[-5].minor.yy100, yymsp[-4].minor.yy45, yymsp[-2].minor.yy102), yymsp[0].minor.yy76);
	}
#line 1128 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 30: /* typeVar ::= type varName */
#line 218 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy121 = AST_MakeTypeVar(yymsp[-1].minor.yy100, yymsp[0].minor.yy45);
	}
#line 1135 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 32: /* typeVarList ::= nonEmptyTypeVarList */
      case 36: /* valueList ::= nonEmptyValueList */ yytestcase(yyruleno==36);
#line 228 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = yymsp[0].minor.yy102;
	}
#line 1143 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 33: /* nonEmptyTypeVarList ::= typeVar */
#line 233 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = AST_ListAppend(AST_ListCreate(), yymsp[0].minor.yy121);
	}
#line 1150 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 34: /* nonEmptyTypeVarList ::= nonEmptyTypeVarList COMMA typeVar */
#line 238 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = AST_ListAppend(yymsp[-2].minor.yy102, yymsp[0].minor.yy121);
	}
#line 1157 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 37: /* nonEmptyValueList ::= value */
#line 253 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = AST_ListAppend(AST_ListCreate(), yymsp[0].minor.yy75);
	}
#line 1164 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 38: /* nonEmptyValueList ::= nonEmptyValueList COMMA value */
#line 258 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = AST_ListAppend(yymsp[-2].minor.yy102, yymsp[0].minor.yy75);
	}
#line 1171 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 39: /* scope ::= LCURLYBRACKET statementList RCURLYBRACKET */
#line 263 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy76 = AST_MakeScope(yymsp[-1].minor.yy102);
	}
#line 1178 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 41: /* statementList ::= statementList statement */
#line 273 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy102 = AST_ListAppend(yymsp[-1].minor.yy102, yymsp[0].minor.yy99);
	}
#line 1185 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 42: /* statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope */
#line 278 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy99 = AST_MakeIf(yymsp[-2].minor.yy75, yymsp[0].minor.yy76, NULL);
	}
#line 1192 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 43: /* statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope */
#line 283 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy99 = AST_MakeIf(yymsp[-4].minor.yy75, yymsp[-2].minor.yy76, yymsp[0].minor.yy76);
	}
#line 1199 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 44: /* statement ::= FOR LROUNDBRACKET varName COLON value RROUNDBRACKET scope */
      case 45: /* statement ::= WHILE LROUNDBRACKET value RROUNDBRACKET scope */ yytestcase(yyruleno==45);
#line 288 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		// TODO
		yygotominor.yy99 = AST_MakeValueStmt(yymsp[-2].minor.yy75);
	}
#line 1208 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 46: /* statement ::= AUTO varName SETEQUAL value SEMICOLON */
#line 300 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy99 = AST_MakeAutoVarDecl(yymsp[-3].minor.yy45, yymsp[-1].minor.yy75);
	}
#line 1215 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 47: /* statement ::= type varName SETEQUAL value SEMICOLON */
#line 305 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy99 = AST_MakeVarDecl(yymsp[-4].minor.yy100, yymsp[-3].minor.yy45, yymsp[-1].minor.yy75);
	}
#line 1222 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 48: /* statement ::= lcName SETEQUAL value SEMICOLON */
#line 310 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy99 = AST_MakeAssignVar(AST_MakeLocalVar(yymsp[-3].minor.yy45), yymsp[-1].minor.yy75);
	}
#line 1229 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 49: /* statement ::= AT lcName SETEQUAL value SEMICOLON */
#line 315 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy99 = AST_MakeAssignVar(AST_MakeThisVar(yymsp[-3].minor.yy45), yymsp[-1].minor.yy75);
	}
#line 1236 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 50: /* statement ::= value SEMICOLON */
#line 320 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy99 = AST_MakeValueStmt(yymsp[-1].minor.yy75);
	}
#line 1243 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 51: /* statement ::= RETURN value SEMICOLON */
#line 325 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy99 = AST_MakeReturn(yymsp[-1].minor.yy75);
	}
#line 1250 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 52: /* precision7 ::= LROUNDBRACKET precision0 RROUNDBRACKET */
#line 330 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = yymsp[-1].minor.yy75;
	}
#line 1257 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 53: /* precision7 ::= lcName */
#line 335 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeVarAccess(AST_MakeLocalVar(yymsp[0].minor.yy45));
	}
#line 1264 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 54: /* precision7 ::= AT lcName */
#line 340 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeVarAccess(AST_MakeThisVar(yymsp[0].minor.yy45));
	}
#line 1271 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 55: /* precision7 ::= BOOLCONSTANT */
#line 345 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBoolConstant((yymsp[0].minor.yy0).value.boolValue);
	}
#line 1278 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 56: /* precision7 ::= INTCONSTANT */
#line 350 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeIntConstant((yymsp[0].minor.yy0).value.intValue);
	}
#line 1285 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 57: /* precision7 ::= FLOATCONSTANT */
#line 355 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeFloatConstant((yymsp[0].minor.yy0).value.floatValue);
	}
#line 1292 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 58: /* precision7 ::= ucName LROUNDBRACKET valueList RROUNDBRACKET */
#line 360 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeConstruct(yymsp[-3].minor.yy45, NULL, yymsp[-1].minor.yy102);
	}
#line 1299 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 59: /* precision7 ::= ucName COLON ucName LROUNDBRACKET valueList RROUNDBRACKET */
#line 365 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeConstruct(yymsp[-5].minor.yy45, yymsp[-3].minor.yy45, yymsp[-1].minor.yy102);
	}
#line 1306 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 60: /* precision6 ::= precision7 */
      case 65: /* precision5 ::= precision6 */ yytestcase(yyruleno==65);
      case 71: /* precision4 ::= precision5 */ yytestcase(yyruleno==71);
      case 74: /* precision3 ::= precision4 */ yytestcase(yyruleno==74);
      case 77: /* precision2 ::= precision3 */ yytestcase(yyruleno==77);
      case 82: /* precision1 ::= precision2 */ yytestcase(yyruleno==82);
      case 84: /* precision0 ::= precision1 */ yytestcase(yyruleno==84);
      case 85: /* value ::= precision0 */ yytestcase(yyruleno==85);
#line 370 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = yymsp[0].minor.yy75;
	}
#line 1320 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 61: /* precision6 ::= precision6 DOT varName */
#line 375 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeMemberAccess(yymsp[-2].minor.yy75, yymsp[0].minor.yy45);
	}
#line 1327 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 62: /* precision6 ::= precision6 DOT methodName LROUNDBRACKET valueList RROUNDBRACKET */
#line 380 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeMethodCall(yymsp[-5].minor.yy75, yymsp[-3].minor.yy45, yymsp[-1].minor.yy102);
	}
#line 1334 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 63: /* precision6 ::= precision6 PTRACCESS varName */
#line 385 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeMemberAccess(AST_MakeUnary(AST_UNARY_DEREF, yymsp[-2].minor.yy75), yymsp[0].minor.yy45);
	}
#line 1341 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 64: /* precision6 ::= precision6 PTRACCESS methodName LROUNDBRACKET valueList RROUNDBRACKET */
#line 390 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeMethodCall(AST_MakeUnary(AST_UNARY_DEREF, yymsp[-5].minor.yy75), yymsp[-3].minor.yy45, yymsp[-1].minor.yy102);
	}
#line 1348 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 66: /* precision5 ::= PLUS precision5 */
#line 400 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeUnary(AST_UNARY_PLUS, yymsp[0].minor.yy75);
	}
#line 1355 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 67: /* precision5 ::= MINUS precision5 */
#line 405 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeUnary(AST_UNARY_MINUS, yymsp[0].minor.yy75);
	}
#line 1362 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 68: /* precision5 ::= EXCLAIMMARK precision5 */
#line 410 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeUnary(AST_UNARY_NEGATE, yymsp[0].minor.yy75);
	}
#line 1369 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 69: /* precision5 ::= AMPERSAND precision5 */
#line 415 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeUnary(AST_UNARY_ADDRESSOF, yymsp[0].minor.yy75);
	}
#line 1376 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 70: /* precision5 ::= STAR precision5 */
#line 420 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeUnary(AST_UNARY_DEREF, yymsp[0].minor.yy75);
	}
#line 1383 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 72: /* precision4 ::= precision4 STAR precision5 */
#line 430 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBinary(AST_BINARY_MULTIPLY, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1390 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 73: /* precision4 ::= precision4 FORWARDSLASH precision5 */
#line 435 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBinary(AST_BINARY_DIVIDE, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1397 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 75: /* precision3 ::= precision3 PLUS precision4 */
#line 445 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBinary(AST_BINARY_ADD, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1404 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 76: /* precision3 ::= precision3 MINUS precision4 */
#line 450 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBinary(AST_BINARY_SUBTRACT, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1411 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 78: /* precision2 ::= precision3 ISEQUAL precision3 */
#line 460 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBinary(AST_BINARY_ISEQUAL, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1418 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 79: /* precision2 ::= precision3 NOTEQUAL precision3 */
#line 465 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBinary(AST_BINARY_NOTEQUAL, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1425 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 80: /* precision2 ::= precision3 GREATEROREQUAL precision3 */
#line 470 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBinary(AST_BINARY_GREATEROREQUAL, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1432 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 81: /* precision2 ::= precision3 LESSOREQUAL precision3 */
#line 475 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeBinary(AST_BINARY_LESSOREQUAL, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1439 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 83: /* precision1 ::= precision2 QUESTIONMARK precision1 COLON precision1 */
#line 485 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy75 = AST_MakeTernary(yymsp[-4].minor.yy75, yymsp[-2].minor.yy75, yymsp[0].minor.yy75);
	}
#line 1446 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      default:
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  Locic_ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 16 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"

	printf("Failure!\n");
#line 1497 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
  Locic_ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  Locic_ParseARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 20 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"

	printf("Syntax error\n");
#line 1515 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
  Locic_ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  Locic_ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
#line 12 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"

	printf("Success!\n");
#line 1537 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
  Locic_ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "Locic_ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Locic_Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  Locic_ParseTOKENTYPE yyminor       /* The value for the token */
  Locic_ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  Locic_ParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
