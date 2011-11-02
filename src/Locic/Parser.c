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
#define YYNOCODE 76
#define YYACTIONTYPE unsigned short int
#define Locic_ParseTOKENTYPE  Locic_Token 
typedef union {
  int yyinit;
  Locic_ParseTOKENTYPE yy0;
  AST_ClassMethodDecl * yy23;
  AST_File * yy28;
  AST_ClassDecl * yy42;
  AST_Scope * yy44;
  AST_Value * yy47;
  AST_ClassDef * yy57;
  AST_ClassMethodDef * yy68;
  AST_TypeVar * yy101;
  char * yy103;
  AST_List * yy112;
  AST_Statement * yy117;
  AST_Type * yy140;
  int yy151;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define Locic_ParseARG_SDECL  AST_File ** resultAST ;
#define Locic_ParseARG_PDECL , AST_File ** resultAST 
#define Locic_ParseARG_FETCH  AST_File ** resultAST  = yypParser->resultAST 
#define Locic_ParseARG_STORE yypParser->resultAST  = resultAST 
#define YYNSTATE 184
#define YYNRULE 85
#define YYERRORSYMBOL 43
#define YYERRSYMDT yy151
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
#define YY_ACTTAB_COUNT (482)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   133,   15,   28,  180,  172,  178,  177,  176,  175,   41,
 /*    10 */    31,    5,   62,   94,   26,   91,   30,   88,   47,  172,
 /*    20 */    50,    6,  156,  155,  154,  119,   55,   35,   34,   33,
 /*    30 */    32,   43,   73,   84,   23,   22,  174,   29,   72,  158,
 /*    40 */    80,  104,  150,  103,  170,  132,  138,  139,  101,   63,
 /*    50 */    70,  143,   71,  151,   72,  158,   69,  143,   71,  151,
 /*    60 */    72,  158,  153,  108,   97,   40,   39,  117,  138,  139,
 /*    70 */   101,   63,   70,  143,   71,  151,   72,  158,   68,  143,
 /*    80 */    71,  151,   45,  179,  153,  108,  105,  174,  167,  168,
 /*    90 */   138,  139,  101,   63,   70,  143,   71,  151,   72,  158,
 /*   100 */   180,   38,  178,  177,  176,  175,  153,  108,  102,  104,
 /*   110 */   137,   99,  138,  139,  101,   63,   70,  143,   71,  151,
 /*   120 */    27,   72,  158,  165,   15,  115,  180,  172,  184,  153,
 /*   130 */   108,   98,   58,   31,  113,  138,  139,  101,   63,   70,
 /*   140 */   143,   71,  151,   56,  164,  156,  155,  154,   72,  158,
 /*   150 */    35,   34,   33,   32,   25,  180,  160,  178,  177,  176,
 /*   160 */   175,   41,  138,  139,  101,   63,   70,  143,   71,  151,
 /*   170 */    72,  158,  171,   90,  181,  171,   86,  180,   95,  178,
 /*   180 */   177,  176,  175,   41,  138,  139,  101,   63,   70,  143,
 /*   190 */    71,  151,   72,  158,  183,  182,  162,  171,  169,  180,
 /*   200 */    93,  178,  177,  176,  175,   41,  138,  139,  101,   63,
 /*   210 */    70,  143,   71,  151,  163,   72,  158,   44,  120,   23,
 /*   220 */    22,  111,  174,   89,   21,   20,   19,   18,   61,  138,
 /*   230 */   139,  101,   63,   70,  143,   71,  151,   72,  158,   57,
 /*   240 */   171,   96,   45,  179,   76,   87,  121,  174,  166,  116,
 /*   250 */   118,  138,  139,  101,   63,   70,  143,   71,  151,   72,
 /*   260 */   158,  165,   46,   78,  122,   42,  110,   85,   72,  158,
 /*   270 */   174,   14,  159,  138,  139,  101,   63,   70,  143,   71,
 /*   280 */   151,  152,   72,  158,    4,   67,   70,  143,   71,  151,
 /*   290 */    83,  179,  270,  195,    3,  173,  138,  139,  101,   63,
 /*   300 */    70,  143,   71,  151,   72,  158,  149,   17,   45,  179,
 /*   310 */    72,  158,   81,  174,  166,  116,  114,   16,  138,  139,
 /*   320 */   101,   63,   70,  143,   71,  151,   72,  158,    2,  148,
 /*   330 */    71,  151,  136,  135,   79,   72,  158,   13,   12,  134,
 /*   340 */   138,  139,  101,   63,   70,  143,   71,  151,   54,  107,
 /*   350 */   139,  101,   63,   70,  143,   71,  151,   72,  158,   53,
 /*   360 */    48,    9,   52,   10,   11,   72,  158,   51,  128,  127,
 /*   370 */     8,   49,  100,  101,   63,   70,  143,   71,  151,  126,
 /*   380 */   140,  101,   63,   70,  143,   71,  151,   72,  158,    7,
 /*   390 */    59,   24,  125,   72,  158,   45,  179,  124,   72,  158,
 /*   400 */   174,  166,  116,  112,   66,   70,  143,   71,  151,  185,
 /*   410 */    65,   70,  143,   71,  151,   64,   70,  143,   71,  151,
 /*   420 */    36,    1,   75,   37,   45,  179,   72,  158,  180,  174,
 /*   430 */   166,  116,  109,   45,  179,   72,  158,   74,  174,  166,
 /*   440 */   116,   77,   60,  157,  161,  147,   71,  151,   72,  158,
 /*   450 */   172,   72,  158,  106,  146,   71,  151,   72,  158,   82,
 /*   460 */    72,  158,  271,   92,  271,  131,  130,  145,   71,  151,
 /*   470 */   144,   71,  151,  129,  123,  271,  142,   71,  151,  141,
 /*   480 */    71,  151,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     6,    7,    7,    9,   10,   11,   12,   13,   14,   15,
 /*    10 */    16,    7,    5,   19,    7,   21,   16,   23,   24,   10,
 /*    20 */    26,   27,   28,   29,   30,   16,   22,   33,   34,   35,
 /*    30 */    36,   51,   52,   53,   33,   34,   56,   37,   52,   53,
 /*    40 */    60,   53,   54,   55,   15,   65,   66,   67,   68,   69,
 /*    50 */    70,   71,   72,   73,   52,   53,   70,   71,   72,   73,
 /*    60 */    52,   53,   60,   61,   62,   31,   32,    8,   66,   67,
 /*    70 */    68,   69,   70,   71,   72,   73,   52,   53,   70,   71,
 /*    80 */    72,   73,   51,   52,   60,   61,   62,   56,   57,   17,
 /*    90 */    66,   67,   68,   69,   70,   71,   72,   73,   52,   53,
 /*   100 */     9,   18,   11,   12,   13,   14,   60,   61,   62,   53,
 /*   110 */    54,   55,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   120 */     7,   52,   53,   53,    7,   55,    9,   10,    0,   60,
 /*   130 */    61,   62,    4,   16,    8,   66,   67,   68,   69,   70,
 /*   140 */    71,   72,   73,   26,   17,   28,   29,   30,   52,   53,
 /*   150 */    33,   34,   35,   36,    7,    9,   60,   11,   12,   13,
 /*   160 */    14,   15,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   170 */    52,   53,   53,   54,    6,   53,   54,    9,   60,   11,
 /*   180 */    12,   13,   14,   15,   66,   67,   68,   69,   70,   71,
 /*   190 */    72,   73,   52,   53,   45,   46,    6,   53,   54,    9,
 /*   200 */    60,   11,   12,   13,   14,   15,   66,   67,   68,   69,
 /*   210 */    70,   71,   72,   73,   48,   52,   53,   51,   52,   33,
 /*   220 */    34,    8,   56,   60,   38,   39,   40,   41,    5,   66,
 /*   230 */    67,   68,   69,   70,   71,   72,   73,   52,   53,    8,
 /*   240 */    53,   54,   51,   52,    1,   60,    3,   56,   57,   58,
 /*   250 */    59,   66,   67,   68,   69,   70,   71,   72,   73,   52,
 /*   260 */    53,   53,   44,   55,   50,   51,   52,   60,   52,   53,
 /*   270 */    56,   18,    8,   66,   67,   68,   69,   70,   71,   72,
 /*   280 */    73,    8,   52,   53,    7,   69,   70,   71,   72,   73,
 /*   290 */    60,   52,   74,    7,    7,   56,   66,   67,   68,   69,
 /*   300 */    70,   71,   72,   73,   52,   53,    8,   42,   51,   52,
 /*   310 */    52,   53,   60,   56,   57,   58,   59,   22,   66,   67,
 /*   320 */    68,   69,   70,   71,   72,   73,   52,   53,    7,   71,
 /*   330 */    72,   73,    8,    8,   60,   52,   53,   25,    7,   17,
 /*   340 */    66,   67,   68,   69,   70,   71,   72,   73,    8,   66,
 /*   350 */    67,   68,   69,   70,   71,   72,   73,   52,   53,   20,
 /*   360 */     7,   25,    8,    7,   22,   52,   53,    8,   17,   17,
 /*   370 */    25,    8,   67,   68,   69,   70,   71,   72,   73,   17,
 /*   380 */    67,   68,   69,   70,   71,   72,   73,   52,   53,   25,
 /*   390 */     2,    7,   17,   52,   53,   51,   52,   17,   52,   53,
 /*   400 */    56,   57,   58,   59,   69,   70,   71,   72,   73,    0,
 /*   410 */    69,   70,   71,   72,   73,   69,   70,   71,   72,   73,
 /*   420 */    49,   64,   43,   47,   51,   52,   52,   53,    9,   56,
 /*   430 */    57,   58,   59,   51,   52,   52,   53,   52,   56,   57,
 /*   440 */    58,   59,    5,   53,   63,   71,   72,   73,   52,   53,
 /*   450 */    10,   52,   53,   52,   71,   72,   73,   52,   53,   53,
 /*   460 */    52,   53,   75,   63,   75,   63,   63,   71,   72,   73,
 /*   470 */    71,   72,   73,   63,   63,   75,   71,   72,   73,   71,
 /*   480 */    72,   73,
};
#define YY_SHIFT_USE_DFLT (-7)
#define YY_SHIFT_COUNT (120)
#define YY_SHIFT_MIN   (-6)
#define YY_SHIFT_MAX   (440)
static const short yy_shift_ofst[] = {
 /*     0 */   243,   -6,  117,  117,  117,  117,  117,  117,  117,  117,
 /*    10 */   117,  117,  117,  117,  117,  117,  117,  117,  117,  117,
 /*    20 */   117,  117,  117,  117,  146,  146,  146,  146,  146,  117,
 /*    30 */   117,  117,  117,  117,  117,  117,  190,  168,  146,  440,
 /*    40 */   440,   91,    9,    9,    9,    9,  128,  440,  440,  437,
 /*    50 */   440,  437,  437,  437,  437,  419,  440,  437,  419,   -7,
 /*    60 */    -7,   -7,   -7,  186,    1,    1,    1,    1,    0,    0,
 /*    70 */     0,   34,    4,    4,    7,  409,  388,  363,  384,  380,
 /*    80 */   375,  362,  364,  352,  345,  351,  336,  359,  356,  354,
 /*    90 */   342,  353,  339,  340,  331,  322,  312,  325,  324,  321,
 /*   100 */   295,  265,  298,  287,  286,  273,  277,  264,  253,  231,
 /*   110 */   147,  223,  213,  127,  126,  113,   83,   72,   59,   29,
 /*   120 */    -5,
};
#define YY_REDUCE_USE_DFLT (-21)
#define YY_REDUCE_COUNT (62)
#define YY_REDUCE_MIN   (-20)
#define YY_REDUCE_MAX   (411)
static const short yy_reduce_ofst[] = {
 /*     0 */   218,  -20,   69,   46,   24,    2,  274,  252,  230,  207,
 /*    10 */   185,  163,  140,  118,   96,  283,  313,  305,  346,  341,
 /*    20 */   335,  216,    8,  -14,  382,  373,  344,  257,  191,  408,
 /*    30 */   405,  399,  396,  383,  374,  258,  214,  166,   31,   56,
 /*    40 */   -12,  239,  208,  187,   70,  144,  149,  122,  119,  411,
 /*    50 */   406,  410,  403,  402,  400,  401,  390,  381,  385,  379,
 /*    60 */   357,  371,  376,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   186,  269,  218,  218,  218,  218,  269,  269,  269,  269,
 /*    10 */   269,  269,  269,  269,  269,  269,  269,  269,  269,  269,
 /*    20 */   269,  269,  269,  269,  214,  214,  214,  214,  214,  269,
 /*    30 */   269,  269,  269,  269,  269,  269,  269,  269,  269,  269,
 /*    40 */   269,  269,  269,  269,  269,  269,  269,  269,  269,  269,
 /*    50 */   269,  269,  269,  269,  269,  269,  269,  269,  269,  269,
 /*    60 */   223,  207,  205,  260,  264,  263,  262,  261,  259,  258,
 /*    70 */   257,  248,  269,  196,  269,  269,  269,  269,  269,  269,
 /*    80 */   269,  269,  237,  269,  236,  269,  269,  269,  269,  269,
 /*    90 */   269,  269,  225,  269,  269,  269,  269,  269,  269,  269,
 /*   100 */   269,  265,  269,  269,  194,  269,  269,  269,  219,  269,
 /*   110 */   196,  269,  269,  269,  269,  269,  215,  269,  269,  203,
 /*   120 */   196,  187,  208,  212,  234,  233,  232,  231,  229,  228,
 /*   130 */   227,  226,  224,  222,  230,  241,  247,  246,  268,  267,
 /*   140 */   266,  256,  255,  254,  253,  252,  251,  250,  249,  245,
 /*   150 */   244,  243,  242,  220,  240,  239,  238,  237,  236,  235,
 /*   160 */   221,  211,  191,  206,  210,  195,  216,  217,  209,  213,
 /*   170 */   204,  194,  193,  202,  201,  200,  199,  198,  197,  196,
 /*   180 */   192,  190,  189,  188,
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
  "$",             "UNKNOWN",       "ERROR",         "INTERFACE",   
  "CLASS",         "LCURLYBRACKET",  "RCURLYBRACKET",  "LROUNDBRACKET",
  "RROUNDBRACKET",  "UCNAME",        "LCNAME",        "VOIDNAME",    
  "BOOLNAME",      "INTNAME",       "FLOATNAME",     "CONST",       
  "STAR",          "SEMICOLON",     "COMMA",         "IF",          
  "ELSE",          "FOR",           "COLON",         "WHILE",       
  "AUTO",          "SETEQUAL",      "AT",            "RETURN",      
  "BOOLCONSTANT",  "INTCONSTANT",   "FLOATCONSTANT",  "DOT",         
  "PTRACCESS",     "PLUS",          "MINUS",         "EXCLAIMMARK", 
  "AMPERSAND",     "FORWARDSLASH",  "ISEQUAL",       "NOTEQUAL",    
  "GREATEROREQUAL",  "LESSOREQUAL",   "QUESTIONMARK",  "error",       
  "file",          "classDecl",     "classDef",      "classMethodDeclList",
  "classMethodDecl",  "classMethodDefList",  "classMethodDef",  "type",        
  "ucName",        "lcName",        "varName",       "methodName",  
  "typeName",      "typeVar",       "nonEmptyTypeVarList",  "typeVarList", 
  "value",         "nonEmptyValueList",  "valueList",     "scope",       
  "statementList",  "statement",     "precision0",    "precision1",  
  "precision2",    "precision3",    "precision4",    "precision5",  
  "precision6",    "precision7",    "start",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "start ::= file",
 /*   1 */ "start ::= UNKNOWN ERROR error",
 /*   2 */ "file ::=",
 /*   3 */ "file ::= INTERFACE",
 /*   4 */ "file ::= file classDecl",
 /*   5 */ "file ::= file classDef",
 /*   6 */ "classDecl ::= CLASS ucName LCURLYBRACKET classMethodDeclList RCURLYBRACKET",
 /*   7 */ "classDef ::= CLASS ucName LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classMethodDefList RCURLYBRACKET",
 /*   8 */ "ucName ::= UCNAME",
 /*   9 */ "lcName ::= LCNAME",
 /*  10 */ "varName ::= lcName",
 /*  11 */ "methodName ::= lcName",
 /*  12 */ "typeName ::= ucName",
 /*  13 */ "typeName ::= VOIDNAME",
 /*  14 */ "typeName ::= BOOLNAME",
 /*  15 */ "typeName ::= INTNAME",
 /*  16 */ "typeName ::= FLOATNAME",
 /*  17 */ "type ::= typeName",
 /*  18 */ "type ::= CONST typeName",
 /*  19 */ "type ::= type STAR",
 /*  20 */ "type ::= type STAR CONST",
 /*  21 */ "classMethodDeclList ::=",
 /*  22 */ "classMethodDeclList ::= classMethodDeclList classMethodDecl",
 /*  23 */ "classMethodDefList ::=",
 /*  24 */ "classMethodDefList ::= classMethodDefList classMethodDef",
 /*  25 */ "classMethodDecl ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON",
 /*  26 */ "classMethodDecl ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON",
 /*  27 */ "classMethodDef ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET scope",
 /*  28 */ "classMethodDef ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET scope",
 /*  29 */ "typeVar ::= type varName",
 /*  30 */ "typeVarList ::=",
 /*  31 */ "typeVarList ::= nonEmptyTypeVarList",
 /*  32 */ "nonEmptyTypeVarList ::= typeVar",
 /*  33 */ "nonEmptyTypeVarList ::= nonEmptyTypeVarList COMMA typeVar",
 /*  34 */ "valueList ::=",
 /*  35 */ "valueList ::= nonEmptyValueList",
 /*  36 */ "nonEmptyValueList ::= value",
 /*  37 */ "nonEmptyValueList ::= nonEmptyValueList COMMA value",
 /*  38 */ "scope ::= LCURLYBRACKET statementList RCURLYBRACKET",
 /*  39 */ "statementList ::=",
 /*  40 */ "statementList ::= statementList statement",
 /*  41 */ "statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope",
 /*  42 */ "statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope",
 /*  43 */ "statement ::= FOR LROUNDBRACKET varName COLON value RROUNDBRACKET scope",
 /*  44 */ "statement ::= WHILE LROUNDBRACKET value RROUNDBRACKET scope",
 /*  45 */ "statement ::= AUTO varName SETEQUAL value SEMICOLON",
 /*  46 */ "statement ::= type varName SETEQUAL value SEMICOLON",
 /*  47 */ "statement ::= lcName SETEQUAL value SEMICOLON",
 /*  48 */ "statement ::= AT lcName SETEQUAL value SEMICOLON",
 /*  49 */ "statement ::= value SEMICOLON",
 /*  50 */ "statement ::= RETURN value SEMICOLON",
 /*  51 */ "precision7 ::= LROUNDBRACKET precision0 RROUNDBRACKET",
 /*  52 */ "precision7 ::= lcName",
 /*  53 */ "precision7 ::= AT lcName",
 /*  54 */ "precision7 ::= BOOLCONSTANT",
 /*  55 */ "precision7 ::= INTCONSTANT",
 /*  56 */ "precision7 ::= FLOATCONSTANT",
 /*  57 */ "precision7 ::= ucName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  58 */ "precision7 ::= ucName COLON ucName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  59 */ "precision6 ::= precision7",
 /*  60 */ "precision6 ::= precision6 DOT varName",
 /*  61 */ "precision6 ::= precision6 DOT methodName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  62 */ "precision6 ::= precision6 PTRACCESS varName",
 /*  63 */ "precision6 ::= precision6 PTRACCESS methodName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  64 */ "precision5 ::= precision6",
 /*  65 */ "precision5 ::= PLUS precision5",
 /*  66 */ "precision5 ::= MINUS precision5",
 /*  67 */ "precision5 ::= EXCLAIMMARK precision5",
 /*  68 */ "precision5 ::= AMPERSAND precision5",
 /*  69 */ "precision5 ::= STAR precision5",
 /*  70 */ "precision4 ::= precision5",
 /*  71 */ "precision4 ::= precision4 STAR precision5",
 /*  72 */ "precision4 ::= precision4 FORWARDSLASH precision5",
 /*  73 */ "precision3 ::= precision4",
 /*  74 */ "precision3 ::= precision3 PLUS precision4",
 /*  75 */ "precision3 ::= precision3 MINUS precision4",
 /*  76 */ "precision2 ::= precision3",
 /*  77 */ "precision2 ::= precision3 ISEQUAL precision3",
 /*  78 */ "precision2 ::= precision3 NOTEQUAL precision3",
 /*  79 */ "precision2 ::= precision3 GREATEROREQUAL precision3",
 /*  80 */ "precision2 ::= precision3 LESSOREQUAL precision3",
 /*  81 */ "precision1 ::= precision2",
 /*  82 */ "precision1 ::= precision2 QUESTIONMARK precision1 COLON precision1",
 /*  83 */ "precision0 ::= precision1",
 /*  84 */ "value ::= precision0",
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
  { 74, 1 },
  { 74, 3 },
  { 44, 0 },
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
#line 65 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		printf("Completed parsing\n");
		*(resultAST) = yymsp[0].minor.yy28;
	}
#line 982 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 2: /* file ::= */
      case 3: /* file ::= INTERFACE */ yytestcase(yyruleno==3);
#line 74 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy28 = AST_MakeFile();
	}
#line 990 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 4: /* file ::= file classDecl */
#line 84 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy28 = AST_FileAddClassDecl(yymsp[-1].minor.yy28, yymsp[0].minor.yy42);
	}
#line 997 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 5: /* file ::= file classDef */
#line 89 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy28 = AST_FileAddClassDef(yymsp[-1].minor.yy28, yymsp[0].minor.yy57);
	}
#line 1004 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 6: /* classDecl ::= CLASS ucName LCURLYBRACKET classMethodDeclList RCURLYBRACKET */
#line 94 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy42 = AST_MakeClassDecl(yymsp[-3].minor.yy103, yymsp[-1].minor.yy112);
	}
#line 1011 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 7: /* classDef ::= CLASS ucName LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classMethodDefList RCURLYBRACKET */
#line 99 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy57 = AST_MakeClassDef(yymsp[-6].minor.yy103, yymsp[-4].minor.yy112, yymsp[-1].minor.yy112);
	}
#line 1018 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 8: /* ucName ::= UCNAME */
      case 9: /* lcName ::= LCNAME */ yytestcase(yyruleno==9);
      case 13: /* typeName ::= VOIDNAME */ yytestcase(yyruleno==13);
      case 14: /* typeName ::= BOOLNAME */ yytestcase(yyruleno==14);
      case 15: /* typeName ::= INTNAME */ yytestcase(yyruleno==15);
      case 16: /* typeName ::= FLOATNAME */ yytestcase(yyruleno==16);
#line 104 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy103 = (yymsp[0].minor.yy0).str;
	}
#line 1030 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 10: /* varName ::= lcName */
      case 11: /* methodName ::= lcName */ yytestcase(yyruleno==11);
      case 12: /* typeName ::= ucName */ yytestcase(yyruleno==12);
#line 114 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy103 = yymsp[0].minor.yy103;
	}
#line 1039 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 17: /* type ::= typeName */
#line 149 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy140 = AST_MakeNamedType(AST_TYPE_MUTABLE, yymsp[0].minor.yy103);
	}
#line 1046 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 18: /* type ::= CONST typeName */
#line 154 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy140 = AST_MakeNamedType(AST_TYPE_CONST, yymsp[0].minor.yy103);
	}
#line 1053 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 19: /* type ::= type STAR */
#line 159 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy140 = AST_MakePtrType(AST_TYPE_MUTABLE, yymsp[-1].minor.yy140);
	}
#line 1060 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 20: /* type ::= type STAR CONST */
#line 164 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy140 = AST_MakePtrType(AST_TYPE_CONST, yymsp[-2].minor.yy140);
	}
#line 1067 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 21: /* classMethodDeclList ::= */
      case 23: /* classMethodDefList ::= */ yytestcase(yyruleno==23);
      case 30: /* typeVarList ::= */ yytestcase(yyruleno==30);
      case 34: /* valueList ::= */ yytestcase(yyruleno==34);
      case 39: /* statementList ::= */ yytestcase(yyruleno==39);
#line 169 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = AST_ListCreate();
	}
#line 1078 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 22: /* classMethodDeclList ::= classMethodDeclList classMethodDecl */
#line 174 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = AST_ListAppend(yymsp[-1].minor.yy112, yymsp[0].minor.yy23);
	}
#line 1085 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 24: /* classMethodDefList ::= classMethodDefList classMethodDef */
#line 184 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = AST_ListAppend(yymsp[-1].minor.yy112, yymsp[0].minor.yy68);
	}
#line 1092 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 25: /* classMethodDecl ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON */
#line 189 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy23 = AST_MakeClassMethodDecl(0, yymsp[-4].minor.yy103, yymsp[-2].minor.yy112);
	}
#line 1099 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 26: /* classMethodDecl ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON */
#line 194 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy23 = AST_MakeClassMethodDecl(yymsp[-5].minor.yy140, yymsp[-4].minor.yy103, yymsp[-2].minor.yy112);
	}
#line 1106 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 27: /* classMethodDef ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET scope */
#line 199 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy68 = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(0, yymsp[-4].minor.yy103, yymsp[-2].minor.yy112), yymsp[0].minor.yy44);
	}
#line 1113 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 28: /* classMethodDef ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET scope */
#line 204 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy68 = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(yymsp[-5].minor.yy140, yymsp[-4].minor.yy103, yymsp[-2].minor.yy112), yymsp[0].minor.yy44);
	}
#line 1120 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 29: /* typeVar ::= type varName */
#line 209 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy101 = AST_MakeTypeVar(yymsp[-1].minor.yy140, yymsp[0].minor.yy103);
	}
#line 1127 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 31: /* typeVarList ::= nonEmptyTypeVarList */
      case 35: /* valueList ::= nonEmptyValueList */ yytestcase(yyruleno==35);
#line 219 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = yymsp[0].minor.yy112;
	}
#line 1135 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 32: /* nonEmptyTypeVarList ::= typeVar */
#line 224 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = AST_ListAppend(AST_ListCreate(), yymsp[0].minor.yy101);
	}
#line 1142 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 33: /* nonEmptyTypeVarList ::= nonEmptyTypeVarList COMMA typeVar */
#line 229 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = AST_ListAppend(yymsp[-2].minor.yy112, yymsp[0].minor.yy101);
	}
#line 1149 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 36: /* nonEmptyValueList ::= value */
#line 244 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = AST_ListAppend(AST_ListCreate(), yymsp[0].minor.yy47);
	}
#line 1156 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 37: /* nonEmptyValueList ::= nonEmptyValueList COMMA value */
#line 249 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = AST_ListAppend(yymsp[-2].minor.yy112, yymsp[0].minor.yy47);
	}
#line 1163 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 38: /* scope ::= LCURLYBRACKET statementList RCURLYBRACKET */
#line 254 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy44 = AST_MakeScope(yymsp[-1].minor.yy112);
	}
#line 1170 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 40: /* statementList ::= statementList statement */
#line 264 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy112 = AST_ListAppend(yymsp[-1].minor.yy112, yymsp[0].minor.yy117);
	}
#line 1177 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 41: /* statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope */
#line 269 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy117 = AST_MakeIf(yymsp[-2].minor.yy47, yymsp[0].minor.yy44, NULL);
	}
#line 1184 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 42: /* statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope */
#line 274 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy117 = AST_MakeIf(yymsp[-4].minor.yy47, yymsp[-2].minor.yy44, yymsp[0].minor.yy44);
	}
#line 1191 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 43: /* statement ::= FOR LROUNDBRACKET varName COLON value RROUNDBRACKET scope */
      case 44: /* statement ::= WHILE LROUNDBRACKET value RROUNDBRACKET scope */ yytestcase(yyruleno==44);
#line 279 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		// TODO
		yygotominor.yy117 = AST_MakeValueStmt(yymsp[-2].minor.yy47);
	}
#line 1200 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 45: /* statement ::= AUTO varName SETEQUAL value SEMICOLON */
#line 291 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy117 = AST_MakeAutoVarDecl(yymsp[-3].minor.yy103, yymsp[-1].minor.yy47);
	}
#line 1207 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 46: /* statement ::= type varName SETEQUAL value SEMICOLON */
#line 296 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy117 = AST_MakeVarDecl(yymsp[-4].minor.yy140, yymsp[-3].minor.yy103, yymsp[-1].minor.yy47);
	}
#line 1214 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 47: /* statement ::= lcName SETEQUAL value SEMICOLON */
#line 301 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy117 = AST_MakeAssignVar(AST_MakeLocalVar(yymsp[-3].minor.yy103), yymsp[-1].minor.yy47);
	}
#line 1221 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 48: /* statement ::= AT lcName SETEQUAL value SEMICOLON */
#line 306 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy117 = AST_MakeAssignVar(AST_MakeThisVar(yymsp[-3].minor.yy103), yymsp[-1].minor.yy47);
	}
#line 1228 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 49: /* statement ::= value SEMICOLON */
#line 311 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy117 = AST_MakeValueStmt(yymsp[-1].minor.yy47);
	}
#line 1235 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 50: /* statement ::= RETURN value SEMICOLON */
#line 316 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy117 = AST_MakeReturn(yymsp[-1].minor.yy47);
	}
#line 1242 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 51: /* precision7 ::= LROUNDBRACKET precision0 RROUNDBRACKET */
#line 321 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = yymsp[-1].minor.yy47;
	}
#line 1249 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 52: /* precision7 ::= lcName */
#line 326 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeVarAccess(AST_MakeLocalVar(yymsp[0].minor.yy103));
	}
#line 1256 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 53: /* precision7 ::= AT lcName */
#line 331 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeVarAccess(AST_MakeThisVar(yymsp[0].minor.yy103));
	}
#line 1263 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 54: /* precision7 ::= BOOLCONSTANT */
#line 336 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBoolConstant((yymsp[0].minor.yy0).boolValue);
	}
#line 1270 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 55: /* precision7 ::= INTCONSTANT */
#line 341 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeIntConstant((yymsp[0].minor.yy0).intValue);
	}
#line 1277 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 56: /* precision7 ::= FLOATCONSTANT */
#line 346 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeFloatConstant((yymsp[0].minor.yy0).floatValue);
	}
#line 1284 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 57: /* precision7 ::= ucName LROUNDBRACKET valueList RROUNDBRACKET */
#line 351 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeConstruct(yymsp[-3].minor.yy103, NULL, yymsp[-1].minor.yy112);
	}
#line 1291 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 58: /* precision7 ::= ucName COLON ucName LROUNDBRACKET valueList RROUNDBRACKET */
#line 356 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeConstruct(yymsp[-5].minor.yy103, yymsp[-3].minor.yy103, yymsp[-1].minor.yy112);
	}
#line 1298 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 59: /* precision6 ::= precision7 */
      case 64: /* precision5 ::= precision6 */ yytestcase(yyruleno==64);
      case 70: /* precision4 ::= precision5 */ yytestcase(yyruleno==70);
      case 73: /* precision3 ::= precision4 */ yytestcase(yyruleno==73);
      case 76: /* precision2 ::= precision3 */ yytestcase(yyruleno==76);
      case 81: /* precision1 ::= precision2 */ yytestcase(yyruleno==81);
      case 83: /* precision0 ::= precision1 */ yytestcase(yyruleno==83);
      case 84: /* value ::= precision0 */ yytestcase(yyruleno==84);
#line 361 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = yymsp[0].minor.yy47;
	}
#line 1312 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 60: /* precision6 ::= precision6 DOT varName */
#line 366 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeMemberAccess(yymsp[-2].minor.yy47, yymsp[0].minor.yy103);
	}
#line 1319 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 61: /* precision6 ::= precision6 DOT methodName LROUNDBRACKET valueList RROUNDBRACKET */
#line 371 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeMethodCall(yymsp[-5].minor.yy47, yymsp[-3].minor.yy103, yymsp[-1].minor.yy112);
	}
#line 1326 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 62: /* precision6 ::= precision6 PTRACCESS varName */
#line 376 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeMemberAccess(AST_MakeUnary(AST_UNARY_DEREF, yymsp[-2].minor.yy47), yymsp[0].minor.yy103);
	}
#line 1333 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 63: /* precision6 ::= precision6 PTRACCESS methodName LROUNDBRACKET valueList RROUNDBRACKET */
#line 381 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeMethodCall(AST_MakeUnary(AST_UNARY_DEREF, yymsp[-5].minor.yy47), yymsp[-3].minor.yy103, yymsp[-1].minor.yy112);
	}
#line 1340 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 65: /* precision5 ::= PLUS precision5 */
#line 391 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeUnary(AST_UNARY_PLUS, yymsp[0].minor.yy47);
	}
#line 1347 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 66: /* precision5 ::= MINUS precision5 */
#line 396 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeUnary(AST_UNARY_MINUS, yymsp[0].minor.yy47);
	}
#line 1354 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 67: /* precision5 ::= EXCLAIMMARK precision5 */
#line 401 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeUnary(AST_UNARY_NEGATE, yymsp[0].minor.yy47);
	}
#line 1361 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 68: /* precision5 ::= AMPERSAND precision5 */
#line 406 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeUnary(AST_UNARY_ADDRESSOF, yymsp[0].minor.yy47);
	}
#line 1368 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 69: /* precision5 ::= STAR precision5 */
#line 411 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeUnary(AST_UNARY_DEREF, yymsp[0].minor.yy47);
	}
#line 1375 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 71: /* precision4 ::= precision4 STAR precision5 */
#line 421 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBinary(AST_BINARY_MULTIPLY, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1382 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 72: /* precision4 ::= precision4 FORWARDSLASH precision5 */
#line 426 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBinary(AST_BINARY_DIVIDE, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1389 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 74: /* precision3 ::= precision3 PLUS precision4 */
#line 436 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBinary(AST_BINARY_ADD, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1396 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 75: /* precision3 ::= precision3 MINUS precision4 */
#line 441 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBinary(AST_BINARY_SUBTRACT, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1403 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 77: /* precision2 ::= precision3 ISEQUAL precision3 */
#line 451 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBinary(AST_BINARY_ISEQUAL, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1410 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 78: /* precision2 ::= precision3 NOTEQUAL precision3 */
#line 456 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBinary(AST_BINARY_NOTEQUAL, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1417 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 79: /* precision2 ::= precision3 GREATEROREQUAL precision3 */
#line 461 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBinary(AST_BINARY_GREATEROREQUAL, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1424 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 80: /* precision2 ::= precision3 LESSOREQUAL precision3 */
#line 466 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeBinary(AST_BINARY_LESSOREQUAL, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1431 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      case 82: /* precision1 ::= precision2 QUESTIONMARK precision1 COLON precision1 */
#line 476 "/home/scross/Projects/locic/dist/src/Locic/Parser.y"
{
		yygotominor.yy47 = AST_MakeTernary(yymsp[-4].minor.yy47, yymsp[-2].minor.yy47, yymsp[0].minor.yy47);
	}
#line 1438 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
        break;
      default:
      /* (1) start ::= UNKNOWN ERROR error */ yytestcase(yyruleno==1);
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
#line 1490 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
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
#line 1508 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
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
#line 1530 "/home/scross/Projects/locic/dist/src/Locic/Parser.c"
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
