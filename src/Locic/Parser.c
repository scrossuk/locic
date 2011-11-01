/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 7 "Parser.y"
#include <assert.h>
#line 8 "Parser.y"
#include <Locic/Token.h>
#line 9 "Parser.y"
#include <Locic/AST.h>
#line 14 "Parser.c"
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
#define YYNOCODE 74
#define YYACTIONTYPE unsigned short int
#define Locic_ParseTOKENTYPE  Locic_Token 
typedef union {
  int yyinit;
  Locic_ParseTOKENTYPE yy0;
  AST_ClassDef * yy21;
  AST_File * yy50;
  AST_ClassMethodDef * yy62;
  AST_Value * yy81;
  AST_ClassMethodDecl * yy85;
  char * yy99;
  AST_Type * yy112;
  AST_TypeVar * yy115;
  AST_ClassDecl * yy124;
  AST_Scope * yy128;
  AST_List * yy134;
  AST_Statement * yy137;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define Locic_ParseARG_SDECL  void * context ;
#define Locic_ParseARG_PDECL , void * context 
#define Locic_ParseARG_FETCH  void * context  = yypParser->context 
#define Locic_ParseARG_STORE yypParser->context  = context 
#define YYNSTATE 181
#define YYNRULE 84
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
#define YY_ACTTAB_COUNT (486)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   130,   15,   28,  177,  169,  175,  174,  173,  172,   41,
 /*    10 */    31,    5,   61,   91,   26,   88,   30,   85,   47,  114,
 /*    20 */    50,    6,  153,  152,  151,  167,   55,   35,   34,   33,
 /*    30 */    32,   43,   72,   81,   40,   39,  171,   29,   71,  155,
 /*    40 */    77,  101,  147,  100,   27,  129,  135,  136,   98,   62,
 /*    50 */    69,  140,   70,  148,   71,  155,   68,  140,   70,  148,
 /*    60 */    45,  176,  150,  105,   94,  171,  164,  165,  135,  136,
 /*    70 */    98,   62,   69,  140,   70,  148,  168,   87,   71,  155,
 /*    80 */   101,  134,   96,   38,   71,  155,  150,  105,  102,   23,
 /*    90 */    22,  110,  135,  136,   98,   62,   69,  140,   70,  148,
 /*   100 */    71,  155,   67,  140,   70,  148,  168,   83,  150,  105,
 /*   110 */    99,  161,  180,  179,  135,  136,   98,   62,   69,  140,
 /*   120 */    70,  148,   71,  155,  181,   15,   58,  177,  169,  108,
 /*   130 */   150,  105,   95,  162,   31,  112,  135,  136,   98,   62,
 /*   140 */    69,  140,   70,  148,   56,   60,  153,  152,  151,   71,
 /*   150 */   155,   35,   34,   33,   32,   25,  177,  157,  175,  174,
 /*   160 */   173,  172,   41,  135,  136,   98,   62,   69,  140,   70,
 /*   170 */   148,   71,  155,  168,  166,  178,  168,   93,  177,   92,
 /*   180 */   175,  174,  173,  172,   41,  135,  136,   98,   62,   69,
 /*   190 */   140,   70,  148,   71,  155,   57,  162,  159,   75,   14,
 /*   200 */   177,   90,  175,  174,  173,  172,   41,  135,  136,   98,
 /*   210 */    62,   69,  140,   70,  148,   71,  155,  176,  156,   23,
 /*   220 */    22,  170,    4,   86,   21,   20,   19,   18,  149,  135,
 /*   230 */   136,   98,   62,   69,  140,   70,  148,   71,  155,  191,
 /*   240 */     3,   45,  176,  146,   17,   84,  171,  163,  113,  115,
 /*   250 */    16,  135,  136,   98,   62,   69,  140,   70,  148,   71,
 /*   260 */   155,    2,   13,   45,  176,  133,  132,   82,  171,  163,
 /*   270 */   113,  111,  131,  135,  136,   98,   62,   69,  140,   70,
 /*   280 */   148,   71,  155,   12,   53,   45,  176,   11,   54,   80,
 /*   290 */   171,  163,  113,  109,   48,  135,  136,   98,   62,   69,
 /*   300 */   140,   70,  148,   71,  155,   52,   10,   45,  176,   51,
 /*   310 */   169,   78,  171,  163,  113,  106,  116,  135,  136,   98,
 /*   320 */    62,   69,  140,   70,  148,   71,  155,  125,   46,   45,
 /*   330 */   176,   71,  155,   76,  171,  163,  113,   74,    9,  135,
 /*   340 */   136,   98,   62,   69,  140,   70,  148,   71,  155,   24,
 /*   350 */   145,   70,  148,    8,   71,  155,  124,  123,  266,    7,
 /*   360 */   122,  104,  136,   98,   62,   69,  140,   70,  148,   97,
 /*   370 */    98,   62,   69,  140,   70,  148,   71,  155,  177,  121,
 /*   380 */   175,  174,  173,  172,   71,  155,   49,   36,   37,    1,
 /*   390 */   177,  137,   98,   62,   69,  140,   70,  148,   71,  155,
 /*   400 */    73,   66,   69,  140,   70,  148,   71,  155,  158,  154,
 /*   410 */    59,  169,   71,  155,   89,   65,   69,  140,   70,  148,
 /*   420 */    71,  155,  103,   64,   69,  140,   70,  148,  128,   63,
 /*   430 */    69,  140,   70,  148,   71,  155,  127,  126,   79,  144,
 /*   440 */    70,  148,   71,  155,  118,   71,  155,  120,   71,  155,
 /*   450 */   267,  267,  267,  143,   70,  148,   71,  155,  267,  267,
 /*   460 */   267,  142,   70,  148,  141,   70,  148,  139,   70,  148,
 /*   470 */   160,  267,  267,   44,  117,  138,   70,  148,  171,  119,
 /*   480 */    42,  107,  267,  267,  267,  171,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     4,    5,    5,    7,    8,    9,   10,   11,   12,   13,
 /*    10 */    14,    5,    3,   17,    5,   19,   14,   21,   22,    6,
 /*    20 */    24,   25,   26,   27,   28,   13,   20,   31,   32,   33,
 /*    30 */    34,   49,   50,   51,   29,   30,   54,   35,   50,   51,
 /*    40 */    58,   51,   52,   53,    5,   63,   64,   65,   66,   67,
 /*    50 */    68,   69,   70,   71,   50,   51,   68,   69,   70,   71,
 /*    60 */    49,   50,   58,   59,   60,   54,   55,   15,   64,   65,
 /*    70 */    66,   67,   68,   69,   70,   71,   51,   52,   50,   51,
 /*    80 */    51,   52,   53,   16,   50,   51,   58,   59,   60,   31,
 /*    90 */    32,    6,   64,   65,   66,   67,   68,   69,   70,   71,
 /*   100 */    50,   51,   68,   69,   70,   71,   51,   52,   58,   59,
 /*   110 */    60,   15,   43,   44,   64,   65,   66,   67,   68,   69,
 /*   120 */    70,   71,   50,   51,    0,    5,    2,    7,    8,    6,
 /*   130 */    58,   59,   60,   51,   14,   53,   64,   65,   66,   67,
 /*   140 */    68,   69,   70,   71,   24,    3,   26,   27,   28,   50,
 /*   150 */    51,   31,   32,   33,   34,    5,    7,   58,    9,   10,
 /*   160 */    11,   12,   13,   64,   65,   66,   67,   68,   69,   70,
 /*   170 */    71,   50,   51,   51,   52,    4,   51,   52,    7,   58,
 /*   180 */     9,   10,   11,   12,   13,   64,   65,   66,   67,   68,
 /*   190 */    69,   70,   71,   50,   51,    6,   51,    4,   53,   16,
 /*   200 */     7,   58,    9,   10,   11,   12,   13,   64,   65,   66,
 /*   210 */    67,   68,   69,   70,   71,   50,   51,   50,    6,   31,
 /*   220 */    32,   54,    5,   58,   36,   37,   38,   39,    6,   64,
 /*   230 */    65,   66,   67,   68,   69,   70,   71,   50,   51,    5,
 /*   240 */     5,   49,   50,    6,   40,   58,   54,   55,   56,   57,
 /*   250 */    20,   64,   65,   66,   67,   68,   69,   70,   71,   50,
 /*   260 */    51,    5,   23,   49,   50,    6,    6,   58,   54,   55,
 /*   270 */    56,   57,   15,   64,   65,   66,   67,   68,   69,   70,
 /*   280 */    71,   50,   51,    5,   18,   49,   50,   20,    6,   58,
 /*   290 */    54,   55,   56,   57,    5,   64,   65,   66,   67,   68,
 /*   300 */    69,   70,   71,   50,   51,    6,    5,   49,   50,    6,
 /*   310 */     8,   58,   54,   55,   56,   57,   14,   64,   65,   66,
 /*   320 */    67,   68,   69,   70,   71,   50,   51,   15,   42,   49,
 /*   330 */    50,   50,   51,   58,   54,   55,   56,   57,   23,   64,
 /*   340 */    65,   66,   67,   68,   69,   70,   71,   50,   51,    5,
 /*   350 */    69,   70,   71,   23,   50,   51,   15,   15,   72,   23,
 /*   360 */    15,   64,   65,   66,   67,   68,   69,   70,   71,   65,
 /*   370 */    66,   67,   68,   69,   70,   71,   50,   51,    7,   15,
 /*   380 */     9,   10,   11,   12,   50,   51,    6,   47,   45,   62,
 /*   390 */     7,   65,   66,   67,   68,   69,   70,   71,   50,   51,
 /*   400 */    50,   67,   68,   69,   70,   71,   50,   51,   61,   51,
 /*   410 */     3,    8,   50,   51,   61,   67,   68,   69,   70,   71,
 /*   420 */    50,   51,   50,   67,   68,   69,   70,   71,   61,   67,
 /*   430 */    68,   69,   70,   71,   50,   51,   61,   61,   51,   69,
 /*   440 */    70,   71,   50,   51,    1,   50,   51,   61,   50,   51,
 /*   450 */    73,   73,   73,   69,   70,   71,   50,   51,   73,   73,
 /*   460 */    73,   69,   70,   71,   69,   70,   71,   69,   70,   71,
 /*   470 */    46,   73,   73,   49,   50,   69,   70,   71,   54,   48,
 /*   480 */    49,   50,   73,   73,   73,   54,
};
#define YY_SHIFT_USE_DFLT (-5)
#define YY_SHIFT_COUNT (117)
#define YY_SHIFT_MIN   (-4)
#define YY_SHIFT_MAX   (443)
static const short yy_shift_ofst[] = {
 /*     0 */   443,   -4,  120,  120,  120,  120,  120,  120,  120,  120,
 /*    10 */   120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
 /*    20 */   120,  120,  120,  120,  149,  149,  149,  149,  149,  120,
 /*    30 */   120,  120,  120,  120,  120,  120,  193,  171,  149,  403,
 /*    40 */   403,  371,  302,  302,  302,  302,  124,  403,  403,  407,
 /*    50 */   403,  407,  407,  407,  407,  383,  403,  407,  383,   -5,
 /*    60 */    -5,   -5,  188,   58,   58,   58,   58,    2,    2,    2,
 /*    70 */     5,    6,    6,    9,  380,  344,  364,  345,  342,  336,
 /*    80 */   341,  330,  312,  315,  303,  301,  299,  267,  289,  266,
 /*    90 */   282,  278,  257,  239,  260,  259,  256,  230,  204,  237,
 /*   100 */   235,  234,  222,  217,  212,  183,  189,  150,  142,  123,
 /*   110 */    96,   85,   39,   67,   52,   13,   12,   -3,
};
#define YY_REDUCE_USE_DFLT (-19)
#define YY_REDUCE_COUNT (61)
#define YY_REDUCE_MIN   (-18)
#define YY_REDUCE_MAX   (431)
static const short yy_reduce_ofst[] = {
 /*     0 */   286,  -18,   72,   50,   28,    4,  275,  253,  231,  209,
 /*    10 */   187,  165,  143,  121,   99,  297,  326,  304,  362,  356,
 /*    20 */   348,  334,   34,  -12,  280,  258,  236,  214,  192,  406,
 /*    30 */   398,  395,  392,  384,  370,  281,  431,  424,   11,   29,
 /*    40 */   -10,  167,  145,  125,   82,  122,   69,   55,   25,  386,
 /*    50 */   387,  376,  375,  367,  353,  372,  358,  347,  350,  327,
 /*    60 */   340,  343,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   182,  265,  214,  214,  214,  214,  265,  265,  265,  265,
 /*    10 */   265,  265,  265,  265,  265,  265,  265,  265,  265,  265,
 /*    20 */   265,  265,  265,  265,  210,  210,  210,  210,  210,  265,
 /*    30 */   265,  265,  265,  265,  265,  265,  265,  265,  265,  265,
 /*    40 */   265,  265,  265,  265,  265,  265,  265,  265,  265,  265,
 /*    50 */   265,  265,  265,  265,  265,  265,  265,  265,  265,  219,
 /*    60 */   203,  201,  256,  260,  259,  258,  257,  255,  254,  253,
 /*    70 */   244,  265,  192,  265,  265,  265,  265,  265,  265,  233,
 /*    80 */   265,  232,  265,  265,  265,  265,  265,  265,  265,  221,
 /*    90 */   265,  265,  265,  265,  265,  265,  265,  265,  261,  265,
 /*   100 */   265,  190,  265,  265,  265,  215,  265,  192,  265,  265,
 /*   110 */   265,  265,  265,  211,  265,  265,  199,  192,  183,  204,
 /*   120 */   208,  230,  229,  228,  227,  225,  224,  223,  222,  220,
 /*   130 */   218,  226,  237,  243,  242,  264,  263,  262,  252,  251,
 /*   140 */   250,  249,  248,  247,  246,  245,  241,  240,  239,  238,
 /*   150 */   216,  236,  235,  234,  233,  232,  231,  217,  207,  187,
 /*   160 */   202,  206,  191,  212,  213,  205,  209,  200,  190,  189,
 /*   170 */   198,  197,  196,  195,  194,  193,  192,  188,  186,  185,
 /*   180 */   184,
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
  "$",             "INTERFACE",     "CLASS",         "LCURLYBRACKET",
  "RCURLYBRACKET",  "LROUNDBRACKET",  "RROUNDBRACKET",  "UCNAME",      
  "LCNAME",        "VOIDNAME",      "BOOLNAME",      "INTNAME",     
  "FLOATNAME",     "CONST",         "STAR",          "SEMICOLON",   
  "COMMA",         "IF",            "ELSE",          "FOR",         
  "COLON",         "WHILE",         "AUTO",          "SETEQUAL",    
  "AT",            "RETURN",        "BOOLCONSTANT",  "INTCONSTANT", 
  "FLOATCONSTANT",  "DOT",           "PTRACCESS",     "PLUS",        
  "MINUS",         "EXCLAIMMARK",   "AMPERSAND",     "FORWARDSLASH",
  "ISEQUAL",       "NOTEQUAL",      "GREATEROREQUAL",  "LESSOREQUAL", 
  "QUESTIONMARK",  "error",         "file",          "classDecl",   
  "classDef",      "classMethodDeclList",  "classMethodDecl",  "classMethodDefList",
  "classMethodDef",  "type",          "ucName",        "lcName",      
  "varName",       "methodName",    "typeName",      "typeVar",     
  "nonEmptyTypeVarList",  "typeVarList",   "value",         "nonEmptyValueList",
  "valueList",     "scope",         "statementList",  "statement",   
  "precision0",    "precision1",    "precision2",    "precision3",  
  "precision4",    "precision5",    "precision6",    "precision7",  
  "start",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "start ::= file",
 /*   1 */ "file ::=",
 /*   2 */ "file ::= INTERFACE",
 /*   3 */ "file ::= file classDecl",
 /*   4 */ "file ::= file classDef",
 /*   5 */ "classDecl ::= CLASS ucName LCURLYBRACKET classMethodDeclList RCURLYBRACKET",
 /*   6 */ "classDef ::= CLASS ucName LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classMethodDefList RCURLYBRACKET",
 /*   7 */ "ucName ::= UCNAME",
 /*   8 */ "lcName ::= LCNAME",
 /*   9 */ "varName ::= lcName",
 /*  10 */ "methodName ::= lcName",
 /*  11 */ "typeName ::= ucName",
 /*  12 */ "typeName ::= VOIDNAME",
 /*  13 */ "typeName ::= BOOLNAME",
 /*  14 */ "typeName ::= INTNAME",
 /*  15 */ "typeName ::= FLOATNAME",
 /*  16 */ "type ::= typeName",
 /*  17 */ "type ::= CONST typeName",
 /*  18 */ "type ::= type STAR",
 /*  19 */ "type ::= type STAR CONST",
 /*  20 */ "classMethodDeclList ::=",
 /*  21 */ "classMethodDeclList ::= classMethodDeclList classMethodDecl",
 /*  22 */ "classMethodDefList ::=",
 /*  23 */ "classMethodDefList ::= classMethodDefList classMethodDef",
 /*  24 */ "classMethodDecl ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON",
 /*  25 */ "classMethodDecl ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON",
 /*  26 */ "classMethodDef ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET scope",
 /*  27 */ "classMethodDef ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET scope",
 /*  28 */ "typeVar ::= type varName",
 /*  29 */ "typeVarList ::=",
 /*  30 */ "typeVarList ::= nonEmptyTypeVarList",
 /*  31 */ "nonEmptyTypeVarList ::= typeVar",
 /*  32 */ "nonEmptyTypeVarList ::= nonEmptyTypeVarList COMMA typeVar",
 /*  33 */ "valueList ::=",
 /*  34 */ "valueList ::= nonEmptyValueList",
 /*  35 */ "nonEmptyValueList ::= value",
 /*  36 */ "nonEmptyValueList ::= nonEmptyValueList COMMA value",
 /*  37 */ "scope ::= LCURLYBRACKET statementList RCURLYBRACKET",
 /*  38 */ "statementList ::=",
 /*  39 */ "statementList ::= statementList statement",
 /*  40 */ "statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope",
 /*  41 */ "statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope",
 /*  42 */ "statement ::= FOR LROUNDBRACKET varName COLON value RROUNDBRACKET scope",
 /*  43 */ "statement ::= WHILE LROUNDBRACKET value RROUNDBRACKET scope",
 /*  44 */ "statement ::= AUTO varName SETEQUAL value SEMICOLON",
 /*  45 */ "statement ::= type varName SETEQUAL value SEMICOLON",
 /*  46 */ "statement ::= lcName SETEQUAL value SEMICOLON",
 /*  47 */ "statement ::= AT lcName SETEQUAL value SEMICOLON",
 /*  48 */ "statement ::= value SEMICOLON",
 /*  49 */ "statement ::= RETURN value SEMICOLON",
 /*  50 */ "precision7 ::= LROUNDBRACKET precision0 RROUNDBRACKET",
 /*  51 */ "precision7 ::= lcName",
 /*  52 */ "precision7 ::= AT lcName",
 /*  53 */ "precision7 ::= BOOLCONSTANT",
 /*  54 */ "precision7 ::= INTCONSTANT",
 /*  55 */ "precision7 ::= FLOATCONSTANT",
 /*  56 */ "precision7 ::= ucName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  57 */ "precision7 ::= ucName COLON ucName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  58 */ "precision6 ::= precision7",
 /*  59 */ "precision6 ::= precision6 DOT varName",
 /*  60 */ "precision6 ::= precision6 DOT methodName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  61 */ "precision6 ::= precision6 PTRACCESS varName",
 /*  62 */ "precision6 ::= precision6 PTRACCESS methodName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  63 */ "precision5 ::= precision6",
 /*  64 */ "precision5 ::= PLUS precision5",
 /*  65 */ "precision5 ::= MINUS precision5",
 /*  66 */ "precision5 ::= EXCLAIMMARK precision5",
 /*  67 */ "precision5 ::= AMPERSAND precision5",
 /*  68 */ "precision5 ::= STAR precision5",
 /*  69 */ "precision4 ::= precision5",
 /*  70 */ "precision4 ::= precision4 STAR precision5",
 /*  71 */ "precision4 ::= precision4 FORWARDSLASH precision5",
 /*  72 */ "precision3 ::= precision4",
 /*  73 */ "precision3 ::= precision3 PLUS precision4",
 /*  74 */ "precision3 ::= precision3 MINUS precision4",
 /*  75 */ "precision2 ::= precision3",
 /*  76 */ "precision2 ::= precision3 ISEQUAL precision3",
 /*  77 */ "precision2 ::= precision3 NOTEQUAL precision3",
 /*  78 */ "precision2 ::= precision3 GREATEROREQUAL precision3",
 /*  79 */ "precision2 ::= precision3 LESSOREQUAL precision3",
 /*  80 */ "precision1 ::= precision2",
 /*  81 */ "precision1 ::= precision2 QUESTIONMARK precision1 COLON precision1",
 /*  82 */ "precision0 ::= precision1",
 /*  83 */ "value ::= precision0",
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
  { 72, 1 },
  { 42, 0 },
  { 42, 1 },
  { 42, 2 },
  { 42, 2 },
  { 43, 5 },
  { 44, 8 },
  { 50, 1 },
  { 51, 1 },
  { 52, 1 },
  { 53, 1 },
  { 54, 1 },
  { 54, 1 },
  { 54, 1 },
  { 54, 1 },
  { 54, 1 },
  { 49, 1 },
  { 49, 2 },
  { 49, 2 },
  { 49, 3 },
  { 45, 0 },
  { 45, 2 },
  { 47, 0 },
  { 47, 2 },
  { 46, 5 },
  { 46, 6 },
  { 48, 5 },
  { 48, 6 },
  { 55, 2 },
  { 57, 0 },
  { 57, 1 },
  { 56, 1 },
  { 56, 3 },
  { 60, 0 },
  { 60, 1 },
  { 59, 1 },
  { 59, 3 },
  { 61, 3 },
  { 62, 0 },
  { 62, 2 },
  { 63, 5 },
  { 63, 7 },
  { 63, 7 },
  { 63, 5 },
  { 63, 5 },
  { 63, 5 },
  { 63, 4 },
  { 63, 5 },
  { 63, 2 },
  { 63, 3 },
  { 71, 3 },
  { 71, 1 },
  { 71, 2 },
  { 71, 1 },
  { 71, 1 },
  { 71, 1 },
  { 71, 4 },
  { 71, 6 },
  { 70, 1 },
  { 70, 3 },
  { 70, 6 },
  { 70, 3 },
  { 70, 6 },
  { 69, 1 },
  { 69, 2 },
  { 69, 2 },
  { 69, 2 },
  { 69, 2 },
  { 69, 2 },
  { 68, 1 },
  { 68, 3 },
  { 68, 3 },
  { 67, 1 },
  { 67, 3 },
  { 67, 3 },
  { 66, 1 },
  { 66, 3 },
  { 66, 3 },
  { 66, 3 },
  { 66, 3 },
  { 65, 1 },
  { 65, 5 },
  { 64, 1 },
  { 58, 1 },
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
#line 52 "Parser.y"
{
		printf("Parsed file successfully");
	}
#line 973 "Parser.c"
        break;
      case 1: /* file ::= */
      case 2: /* file ::= INTERFACE */ yytestcase(yyruleno==2);
#line 57 "Parser.y"
{
		yygotominor.yy50 = AST_MakeFile();
	}
#line 981 "Parser.c"
        break;
      case 3: /* file ::= file classDecl */
#line 67 "Parser.y"
{
		yygotominor.yy50 = AST_FileAddClassDecl(yymsp[-1].minor.yy50, yymsp[0].minor.yy124);
	}
#line 988 "Parser.c"
        break;
      case 4: /* file ::= file classDef */
#line 72 "Parser.y"
{
		yygotominor.yy50 = AST_FileAddClassDef(yymsp[-1].minor.yy50, yymsp[0].minor.yy21);
	}
#line 995 "Parser.c"
        break;
      case 5: /* classDecl ::= CLASS ucName LCURLYBRACKET classMethodDeclList RCURLYBRACKET */
#line 77 "Parser.y"
{
		yygotominor.yy124 = AST_MakeClassDecl(yymsp[-3].minor.yy99, yymsp[-1].minor.yy134);
	}
#line 1002 "Parser.c"
        break;
      case 6: /* classDef ::= CLASS ucName LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classMethodDefList RCURLYBRACKET */
#line 82 "Parser.y"
{
		yygotominor.yy21 = AST_MakeClassDef(yymsp[-6].minor.yy99, yymsp[-4].minor.yy134, yymsp[-1].minor.yy134);
	}
#line 1009 "Parser.c"
        break;
      case 7: /* ucName ::= UCNAME */
      case 8: /* lcName ::= LCNAME */ yytestcase(yyruleno==8);
      case 12: /* typeName ::= VOIDNAME */ yytestcase(yyruleno==12);
      case 13: /* typeName ::= BOOLNAME */ yytestcase(yyruleno==13);
      case 14: /* typeName ::= INTNAME */ yytestcase(yyruleno==14);
      case 15: /* typeName ::= FLOATNAME */ yytestcase(yyruleno==15);
#line 87 "Parser.y"
{
		yygotominor.yy99 = (yymsp[0].minor.yy0).value.str;
	}
#line 1021 "Parser.c"
        break;
      case 9: /* varName ::= lcName */
      case 10: /* methodName ::= lcName */ yytestcase(yyruleno==10);
      case 11: /* typeName ::= ucName */ yytestcase(yyruleno==11);
#line 97 "Parser.y"
{
		yygotominor.yy99 = yymsp[0].minor.yy99;
	}
#line 1030 "Parser.c"
        break;
      case 16: /* type ::= typeName */
#line 132 "Parser.y"
{
		yygotominor.yy112 = AST_MakeNamedType(AST_TYPE_MUTABLE, yymsp[0].minor.yy99);
	}
#line 1037 "Parser.c"
        break;
      case 17: /* type ::= CONST typeName */
#line 137 "Parser.y"
{
		yygotominor.yy112 = AST_MakeNamedType(AST_TYPE_CONST, yymsp[0].minor.yy99);
	}
#line 1044 "Parser.c"
        break;
      case 18: /* type ::= type STAR */
#line 142 "Parser.y"
{
		yygotominor.yy112 = AST_MakePtrType(AST_TYPE_MUTABLE, yymsp[-1].minor.yy112);
	}
#line 1051 "Parser.c"
        break;
      case 19: /* type ::= type STAR CONST */
#line 147 "Parser.y"
{
		yygotominor.yy112 = AST_MakePtrType(AST_TYPE_CONST, yymsp[-2].minor.yy112);
	}
#line 1058 "Parser.c"
        break;
      case 20: /* classMethodDeclList ::= */
      case 22: /* classMethodDefList ::= */ yytestcase(yyruleno==22);
      case 29: /* typeVarList ::= */ yytestcase(yyruleno==29);
      case 33: /* valueList ::= */ yytestcase(yyruleno==33);
      case 38: /* statementList ::= */ yytestcase(yyruleno==38);
#line 152 "Parser.y"
{
		yygotominor.yy134 = AST_ListCreate();
	}
#line 1069 "Parser.c"
        break;
      case 21: /* classMethodDeclList ::= classMethodDeclList classMethodDecl */
#line 157 "Parser.y"
{
		yygotominor.yy134 = AST_ListAppend(yymsp[-1].minor.yy134, yymsp[0].minor.yy85);
	}
#line 1076 "Parser.c"
        break;
      case 23: /* classMethodDefList ::= classMethodDefList classMethodDef */
#line 167 "Parser.y"
{
		yygotominor.yy134 = AST_ListAppend(yymsp[-1].minor.yy134, yymsp[0].minor.yy62);
	}
#line 1083 "Parser.c"
        break;
      case 24: /* classMethodDecl ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON */
#line 172 "Parser.y"
{
		yygotominor.yy85 = AST_MakeClassMethodDecl(0, yymsp[-4].minor.yy99, yymsp[-2].minor.yy134);
	}
#line 1090 "Parser.c"
        break;
      case 25: /* classMethodDecl ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON */
#line 177 "Parser.y"
{
		yygotominor.yy85 = AST_MakeClassMethodDecl(yymsp[-5].minor.yy112, yymsp[-4].minor.yy99, yymsp[-2].minor.yy134);
	}
#line 1097 "Parser.c"
        break;
      case 26: /* classMethodDef ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET scope */
#line 182 "Parser.y"
{
		yygotominor.yy62 = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(0, yymsp[-4].minor.yy99, yymsp[-2].minor.yy134), yymsp[0].minor.yy128);
	}
#line 1104 "Parser.c"
        break;
      case 27: /* classMethodDef ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET scope */
#line 187 "Parser.y"
{
		yygotominor.yy62 = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(yymsp[-5].minor.yy112, yymsp[-4].minor.yy99, yymsp[-2].minor.yy134), yymsp[0].minor.yy128);
	}
#line 1111 "Parser.c"
        break;
      case 28: /* typeVar ::= type varName */
#line 192 "Parser.y"
{
		yygotominor.yy115 = AST_MakeTypeVar(yymsp[-1].minor.yy112, yymsp[0].minor.yy99);
	}
#line 1118 "Parser.c"
        break;
      case 30: /* typeVarList ::= nonEmptyTypeVarList */
      case 34: /* valueList ::= nonEmptyValueList */ yytestcase(yyruleno==34);
#line 202 "Parser.y"
{
		yygotominor.yy134 = yymsp[0].minor.yy134;
	}
#line 1126 "Parser.c"
        break;
      case 31: /* nonEmptyTypeVarList ::= typeVar */
#line 207 "Parser.y"
{
		yygotominor.yy134 = AST_ListAppend(AST_ListCreate(), yymsp[0].minor.yy115);
	}
#line 1133 "Parser.c"
        break;
      case 32: /* nonEmptyTypeVarList ::= nonEmptyTypeVarList COMMA typeVar */
#line 212 "Parser.y"
{
		yygotominor.yy134 = AST_ListAppend(yymsp[-2].minor.yy134, yymsp[0].minor.yy115);
	}
#line 1140 "Parser.c"
        break;
      case 35: /* nonEmptyValueList ::= value */
#line 227 "Parser.y"
{
		yygotominor.yy134 = AST_ListAppend(AST_ListCreate(), yymsp[0].minor.yy81);
	}
#line 1147 "Parser.c"
        break;
      case 36: /* nonEmptyValueList ::= nonEmptyValueList COMMA value */
#line 232 "Parser.y"
{
		yygotominor.yy134 = AST_ListAppend(yymsp[-2].minor.yy134, yymsp[0].minor.yy81);
	}
#line 1154 "Parser.c"
        break;
      case 37: /* scope ::= LCURLYBRACKET statementList RCURLYBRACKET */
#line 237 "Parser.y"
{
		yygotominor.yy128 = AST_MakeScope(yymsp[-1].minor.yy134);
	}
#line 1161 "Parser.c"
        break;
      case 39: /* statementList ::= statementList statement */
#line 247 "Parser.y"
{
		yygotominor.yy134 = AST_ListAppend(yymsp[-1].minor.yy134, yymsp[0].minor.yy137);
	}
#line 1168 "Parser.c"
        break;
      case 40: /* statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope */
#line 252 "Parser.y"
{
		yygotominor.yy137 = AST_MakeIf(yymsp[-2].minor.yy81, yymsp[0].minor.yy128, NULL);
	}
#line 1175 "Parser.c"
        break;
      case 41: /* statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope */
#line 257 "Parser.y"
{
		yygotominor.yy137 = AST_MakeIf(yymsp[-4].minor.yy81, yymsp[-2].minor.yy128, yymsp[0].minor.yy128);
	}
#line 1182 "Parser.c"
        break;
      case 42: /* statement ::= FOR LROUNDBRACKET varName COLON value RROUNDBRACKET scope */
      case 43: /* statement ::= WHILE LROUNDBRACKET value RROUNDBRACKET scope */ yytestcase(yyruleno==43);
#line 262 "Parser.y"
{
		// TODO
		yygotominor.yy137 = AST_MakeValueStmt(yymsp[-2].minor.yy81);
	}
#line 1191 "Parser.c"
        break;
      case 44: /* statement ::= AUTO varName SETEQUAL value SEMICOLON */
#line 274 "Parser.y"
{
		yygotominor.yy137 = AST_MakeAutoVarDecl(yymsp[-3].minor.yy99, yymsp[-1].minor.yy81);
	}
#line 1198 "Parser.c"
        break;
      case 45: /* statement ::= type varName SETEQUAL value SEMICOLON */
#line 279 "Parser.y"
{
		yygotominor.yy137 = AST_MakeVarDecl(yymsp[-4].minor.yy112, yymsp[-3].minor.yy99, yymsp[-1].minor.yy81);
	}
#line 1205 "Parser.c"
        break;
      case 46: /* statement ::= lcName SETEQUAL value SEMICOLON */
#line 284 "Parser.y"
{
		yygotominor.yy137 = AST_MakeAssignVar(AST_MakeLocalVar(yymsp[-3].minor.yy99), yymsp[-1].minor.yy81);
	}
#line 1212 "Parser.c"
        break;
      case 47: /* statement ::= AT lcName SETEQUAL value SEMICOLON */
#line 289 "Parser.y"
{
		yygotominor.yy137 = AST_MakeAssignVar(AST_MakeThisVar(yymsp[-3].minor.yy99), yymsp[-1].minor.yy81);
	}
#line 1219 "Parser.c"
        break;
      case 48: /* statement ::= value SEMICOLON */
#line 294 "Parser.y"
{
		yygotominor.yy137 = AST_MakeValueStmt(yymsp[-1].minor.yy81);
	}
#line 1226 "Parser.c"
        break;
      case 49: /* statement ::= RETURN value SEMICOLON */
#line 299 "Parser.y"
{
		yygotominor.yy137 = AST_MakeReturn(yymsp[-1].minor.yy81);
	}
#line 1233 "Parser.c"
        break;
      case 50: /* precision7 ::= LROUNDBRACKET precision0 RROUNDBRACKET */
#line 304 "Parser.y"
{
		yygotominor.yy81 = yymsp[-1].minor.yy81;
	}
#line 1240 "Parser.c"
        break;
      case 51: /* precision7 ::= lcName */
#line 309 "Parser.y"
{
		yygotominor.yy81 = AST_MakeVarAccess(AST_MakeLocalVar(yymsp[0].minor.yy99));
	}
#line 1247 "Parser.c"
        break;
      case 52: /* precision7 ::= AT lcName */
#line 314 "Parser.y"
{
		yygotominor.yy81 = AST_MakeVarAccess(AST_MakeThisVar(yymsp[0].minor.yy99));
	}
#line 1254 "Parser.c"
        break;
      case 53: /* precision7 ::= BOOLCONSTANT */
#line 319 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBoolConstant((yymsp[0].minor.yy0).value.boolValue);
	}
#line 1261 "Parser.c"
        break;
      case 54: /* precision7 ::= INTCONSTANT */
#line 324 "Parser.y"
{
		yygotominor.yy81 = AST_MakeIntConstant((yymsp[0].minor.yy0).value.intValue);
	}
#line 1268 "Parser.c"
        break;
      case 55: /* precision7 ::= FLOATCONSTANT */
#line 329 "Parser.y"
{
		yygotominor.yy81 = AST_MakeFloatConstant((yymsp[0].minor.yy0).value.floatValue);
	}
#line 1275 "Parser.c"
        break;
      case 56: /* precision7 ::= ucName LROUNDBRACKET valueList RROUNDBRACKET */
#line 334 "Parser.y"
{
		yygotominor.yy81 = AST_MakeConstruct(yymsp[-3].minor.yy99, NULL, yymsp[-1].minor.yy134);
	}
#line 1282 "Parser.c"
        break;
      case 57: /* precision7 ::= ucName COLON ucName LROUNDBRACKET valueList RROUNDBRACKET */
#line 339 "Parser.y"
{
		yygotominor.yy81 = AST_MakeConstruct(yymsp[-5].minor.yy99, yymsp[-3].minor.yy99, yymsp[-1].minor.yy134);
	}
#line 1289 "Parser.c"
        break;
      case 58: /* precision6 ::= precision7 */
      case 63: /* precision5 ::= precision6 */ yytestcase(yyruleno==63);
      case 69: /* precision4 ::= precision5 */ yytestcase(yyruleno==69);
      case 72: /* precision3 ::= precision4 */ yytestcase(yyruleno==72);
      case 75: /* precision2 ::= precision3 */ yytestcase(yyruleno==75);
      case 80: /* precision1 ::= precision2 */ yytestcase(yyruleno==80);
      case 82: /* precision0 ::= precision1 */ yytestcase(yyruleno==82);
      case 83: /* value ::= precision0 */ yytestcase(yyruleno==83);
#line 344 "Parser.y"
{
		yygotominor.yy81 = yymsp[0].minor.yy81;
	}
#line 1303 "Parser.c"
        break;
      case 59: /* precision6 ::= precision6 DOT varName */
#line 349 "Parser.y"
{
		yygotominor.yy81 = AST_MakeMemberAccess(yymsp[-2].minor.yy81, yymsp[0].minor.yy99);
	}
#line 1310 "Parser.c"
        break;
      case 60: /* precision6 ::= precision6 DOT methodName LROUNDBRACKET valueList RROUNDBRACKET */
#line 354 "Parser.y"
{
		yygotominor.yy81 = AST_MakeMethodCall(yymsp[-5].minor.yy81, yymsp[-3].minor.yy99, yymsp[-1].minor.yy134);
	}
#line 1317 "Parser.c"
        break;
      case 61: /* precision6 ::= precision6 PTRACCESS varName */
#line 359 "Parser.y"
{
		yygotominor.yy81 = AST_MakeMemberAccess(AST_MakeUnary(AST_UNARY_DEREF, yymsp[-2].minor.yy81), yymsp[0].minor.yy99);
	}
#line 1324 "Parser.c"
        break;
      case 62: /* precision6 ::= precision6 PTRACCESS methodName LROUNDBRACKET valueList RROUNDBRACKET */
#line 364 "Parser.y"
{
		yygotominor.yy81 = AST_MakeMethodCall(AST_MakeUnary(AST_UNARY_DEREF, yymsp[-5].minor.yy81), yymsp[-3].minor.yy99, yymsp[-1].minor.yy134);
	}
#line 1331 "Parser.c"
        break;
      case 64: /* precision5 ::= PLUS precision5 */
#line 374 "Parser.y"
{
		yygotominor.yy81 = AST_MakeUnary(AST_UNARY_PLUS, yymsp[0].minor.yy81);
	}
#line 1338 "Parser.c"
        break;
      case 65: /* precision5 ::= MINUS precision5 */
#line 379 "Parser.y"
{
		yygotominor.yy81 = AST_MakeUnary(AST_UNARY_MINUS, yymsp[0].minor.yy81);
	}
#line 1345 "Parser.c"
        break;
      case 66: /* precision5 ::= EXCLAIMMARK precision5 */
#line 384 "Parser.y"
{
		yygotominor.yy81 = AST_MakeUnary(AST_UNARY_NEGATE, yymsp[0].minor.yy81);
	}
#line 1352 "Parser.c"
        break;
      case 67: /* precision5 ::= AMPERSAND precision5 */
#line 389 "Parser.y"
{
		yygotominor.yy81 = AST_MakeUnary(AST_UNARY_ADDRESSOF, yymsp[0].minor.yy81);
	}
#line 1359 "Parser.c"
        break;
      case 68: /* precision5 ::= STAR precision5 */
#line 394 "Parser.y"
{
		yygotominor.yy81 = AST_MakeUnary(AST_UNARY_DEREF, yymsp[0].minor.yy81);
	}
#line 1366 "Parser.c"
        break;
      case 70: /* precision4 ::= precision4 STAR precision5 */
#line 404 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBinary(AST_BINARY_MULTIPLY, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1373 "Parser.c"
        break;
      case 71: /* precision4 ::= precision4 FORWARDSLASH precision5 */
#line 409 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBinary(AST_BINARY_DIVIDE, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1380 "Parser.c"
        break;
      case 73: /* precision3 ::= precision3 PLUS precision4 */
#line 419 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBinary(AST_BINARY_ADD, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1387 "Parser.c"
        break;
      case 74: /* precision3 ::= precision3 MINUS precision4 */
#line 424 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBinary(AST_BINARY_SUBTRACT, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1394 "Parser.c"
        break;
      case 76: /* precision2 ::= precision3 ISEQUAL precision3 */
#line 434 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBinary(AST_BINARY_ISEQUAL, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1401 "Parser.c"
        break;
      case 77: /* precision2 ::= precision3 NOTEQUAL precision3 */
#line 439 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBinary(AST_BINARY_NOTEQUAL, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1408 "Parser.c"
        break;
      case 78: /* precision2 ::= precision3 GREATEROREQUAL precision3 */
#line 444 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBinary(AST_BINARY_GREATEROREQUAL, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1415 "Parser.c"
        break;
      case 79: /* precision2 ::= precision3 LESSOREQUAL precision3 */
#line 449 "Parser.y"
{
		yygotominor.yy81 = AST_MakeBinary(AST_BINARY_LESSOREQUAL, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1422 "Parser.c"
        break;
      case 81: /* precision1 ::= precision2 QUESTIONMARK precision1 COLON precision1 */
#line 459 "Parser.y"
{
		yygotominor.yy81 = AST_MakeTernary(yymsp[-4].minor.yy81, yymsp[-2].minor.yy81, yymsp[0].minor.yy81);
	}
#line 1429 "Parser.c"
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
