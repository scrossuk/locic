/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 7 "Parser.y"
#include <Locic/Token.h>
#line 8 "Parser.y"
#include <Locic/AST.h>
#line 12 "Parser.c"
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
#define YYNOCODE 70
#define YYACTIONTYPE unsigned short int
#define Locic_ParseTOKENTYPE  Locic_Token 
typedef union {
  int yyinit;
  Locic_ParseTOKENTYPE yy0;
  AST_ClassDef * yy15;
  AST_Value * yy17;
  AST_Scope * yy26;
  AST_TypeVar * yy29;
  AST_File * yy70;
  AST_List * yy82;
  char * yy91;
  AST_Statement * yy99;
  AST_Type * yy104;
  AST_ClassDecl * yy108;
  AST_ClassMethodDecl * yy131;
  AST_ClassMethodDef * yy134;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define Locic_ParseARG_SDECL  void * context ;
#define Locic_ParseARG_PDECL , void * context 
#define Locic_ParseARG_FETCH  void * context  = yypParser->context 
#define Locic_ParseARG_STORE yypParser->context  = context 
#define YYNSTATE 174
#define YYNRULE 80
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
#define YY_ACTTAB_COUNT (466)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   123,   15,   25,  170,  162,  168,  167,  166,  165,   38,
 /*    10 */    28,   20,   19,   85,   27,   82,   18,   79,   44,  160,
 /*    20 */    47,    6,  146,  145,  144,   20,   19,   32,   31,   30,
 /*    30 */    29,   40,  112,   39,  164,   26,  164,  170,   71,  168,
 /*    40 */   167,  166,  165,  122,  128,  129,   92,   59,   63,  133,
 /*    50 */    64,  141,  108,   66,   75,  101,  143,   99,   88,  158,
 /*    60 */    37,   36,  128,  129,   92,   59,   63,  133,   64,  141,
 /*    70 */   104,   65,  148,  143,   99,   96,   58,  163,   23,  128,
 /*    80 */   129,   92,   59,   63,  133,   64,  141,   35,   65,  148,
 /*    90 */     5,  143,   99,   93,  173,  172,  169,  128,  129,   92,
 /*   100 */    59,   63,  133,   64,  141,   52,   65,  148,   81,  143,
 /*   110 */    99,   89,  140,   94,   24,  128,  129,   92,   59,   63,
 /*   120 */   133,   64,  141,  106,   65,  148,   15,  154,  170,  162,
 /*   130 */   161,  174,   55,  171,   95,   28,  170,  102,  168,  167,
 /*   140 */   166,  165,   38,   57,  155,   53,   77,  146,  145,  144,
 /*   150 */   150,   22,   32,   31,   30,   29,  128,  129,   92,   59,
 /*   160 */    63,  133,   64,  141,   86,   65,  148,   54,  161,   14,
 /*   170 */   128,  129,   92,   59,   63,  133,   64,  141,   84,   65,
 /*   180 */   148,  149,    4,  183,  128,  129,   92,   59,   63,  133,
 /*   190 */    64,  141,  142,   65,  148,  127,   90,  170,   80,  168,
 /*   200 */   167,  166,  165,   38,  128,  129,   92,   59,   63,  133,
 /*   210 */    64,  141,   78,   65,  148,   43,    3,   95,  128,  129,
 /*   220 */    92,   59,   63,  133,   64,  141,   76,   65,  148,  139,
 /*   230 */    17,    2,  128,  129,   92,   59,   63,  133,   64,  141,
 /*   240 */    74,   65,  148,  255,   16,  126,  128,  129,   92,   59,
 /*   250 */    63,  133,   64,  141,  159,   65,  148,   72,   13,  125,
 /*   260 */   124,   12,   50,  128,  129,   92,   59,   63,  133,   64,
 /*   270 */   141,   70,   65,  148,   51,   45,  161,  128,  129,   92,
 /*   280 */    59,   63,  133,   64,  141,   11,   65,  148,   69,   49,
 /*   290 */    10,   98,  129,   92,   59,   63,  133,   64,  141,   48,
 /*   300 */    65,  148,   91,   92,   59,   63,  133,   64,  141,  155,
 /*   310 */    65,  148,  130,   92,   59,   63,  133,   64,  141,  152,
 /*   320 */    65,  148,  170,    9,  168,  167,  166,  165,   38,   60,
 /*   330 */    63,  133,   64,  141,    8,   65,  148,   42,  118,  162,
 /*   340 */   164,  156,  107,  109,   42,  110,   21,  164,  156,  107,
 /*   350 */   105,   42,  117,   87,  164,  156,  107,  103,   42,  169,
 /*   360 */   116,  164,  156,  107,  100,   42,  169,    7,  164,  156,
 /*   370 */   107,   68,  115,  169,  114,  161,   62,  133,   64,  141,
 /*   380 */   169,   65,  148,   61,  133,   64,  141,  169,   65,  148,
 /*   390 */   138,   64,  141,   46,   65,  148,  137,   64,  141,   33,
 /*   400 */    65,  148,   34,    1,  136,   64,  141,   67,   65,  148,
 /*   410 */   170,  135,   64,  141,  151,   65,  148,   56,  134,   64,
 /*   420 */   141,  147,   65,  148,  162,  132,   64,  141,   83,   65,
 /*   430 */   148,   97,  131,   64,  141,   42,   65,  148,  164,  157,
 /*   440 */   153,  121,  120,   41,  119,   73,  164,  113,  256,  256,
 /*   450 */   256,  256,  256,  256,  256,  256,  256,  169,  256,  256,
 /*   460 */   256,  256,  256,  256,  256,  111,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     3,    4,    4,    6,    7,    8,    9,   10,   11,   12,
 /*    10 */    13,   30,   31,   16,   13,   18,   35,   20,   21,   12,
 /*    20 */    23,   24,   25,   26,   27,   30,   31,   30,   31,   32,
 /*    30 */    33,   45,   44,   45,   48,   34,   48,    6,   52,    8,
 /*    40 */     9,   10,   11,   57,   58,   59,   60,   61,   62,   63,
 /*    50 */    64,   65,    5,   67,   68,   67,   52,   53,   54,   14,
 /*    60 */    28,   29,   58,   59,   60,   61,   62,   63,   64,   65,
 /*    70 */     5,   67,   68,   52,   53,   54,    2,   48,    4,   58,
 /*    80 */    59,   60,   61,   62,   63,   64,   65,   15,   67,   68,
 /*    90 */     4,   52,   53,   54,   39,   40,   67,   58,   59,   60,
 /*   100 */    61,   62,   63,   64,   65,   19,   67,   68,   46,   52,
 /*   110 */    53,   54,   46,   47,    4,   58,   59,   60,   61,   62,
 /*   120 */    63,   64,   65,   47,   67,   68,    4,   14,    6,    7,
 /*   130 */    68,    0,    1,    3,   68,   13,    6,    5,    8,    9,
 /*   140 */    10,   11,   12,    2,   68,   23,   46,   25,   26,   27,
 /*   150 */    52,    4,   30,   31,   32,   33,   58,   59,   60,   61,
 /*   160 */    62,   63,   64,   65,   52,   67,   68,    5,   68,   15,
 /*   170 */    58,   59,   60,   61,   62,   63,   64,   65,   52,   67,
 /*   180 */    68,    5,    4,    4,   58,   59,   60,   61,   62,   63,
 /*   190 */    64,   65,    5,   67,   68,   46,   47,    6,   52,    8,
 /*   200 */     9,   10,   11,   12,   58,   59,   60,   61,   62,   63,
 /*   210 */    64,   65,   52,   67,   68,   38,    4,   68,   58,   59,
 /*   220 */    60,   61,   62,   63,   64,   65,   52,   67,   68,    5,
 /*   230 */    36,    4,   58,   59,   60,   61,   62,   63,   64,   65,
 /*   240 */    52,   67,   68,   66,   19,    5,   58,   59,   60,   61,
 /*   250 */    62,   63,   64,   65,   46,   67,   68,   52,   22,    5,
 /*   260 */    14,    4,   17,   58,   59,   60,   61,   62,   63,   64,
 /*   270 */    65,   52,   67,   68,    5,    4,   68,   58,   59,   60,
 /*   280 */    61,   62,   63,   64,   65,   19,   67,   68,   47,    5,
 /*   290 */     4,   58,   59,   60,   61,   62,   63,   64,   65,    5,
 /*   300 */    67,   68,   59,   60,   61,   62,   63,   64,   65,   68,
 /*   310 */    67,   68,   59,   60,   61,   62,   63,   64,   65,    3,
 /*   320 */    67,   68,    6,   22,    8,    9,   10,   11,   12,   61,
 /*   330 */    62,   63,   64,   65,   22,   67,   68,   45,   14,    7,
 /*   340 */    48,   49,   50,   51,   45,   13,    4,   48,   49,   50,
 /*   350 */    51,   45,   14,   46,   48,   49,   50,   51,   45,   67,
 /*   360 */    14,   48,   49,   50,   51,   45,   67,   22,   48,   49,
 /*   370 */    50,   51,   14,   67,   14,   68,   62,   63,   64,   65,
 /*   380 */    67,   67,   68,   62,   63,   64,   65,   67,   67,   68,
 /*   390 */    63,   64,   65,    5,   67,   68,   63,   64,   65,   43,
 /*   400 */    67,   68,   41,   56,   63,   64,   65,   67,   67,   68,
 /*   410 */     6,   63,   64,   65,   55,   67,   68,    2,   63,   64,
 /*   420 */    65,   68,   67,   68,    7,   63,   64,   65,   55,   67,
 /*   430 */    68,   67,   63,   64,   65,   45,   67,   68,   48,   49,
 /*   440 */    42,   55,   55,   45,   55,   68,   48,   55,   69,   69,
 /*   450 */    69,   69,   69,   69,   69,   69,   69,   67,   69,   69,
 /*   460 */    69,   69,   69,   69,   69,   67,
};
#define YY_SHIFT_USE_DFLT (-20)
#define YY_SHIFT_COUNT (111)
#define YY_SHIFT_MIN   (-19)
#define YY_SHIFT_MAX   (417)
static const short yy_shift_ofst[] = {
 /*     0 */   -20,   -3,  122,  122,  122,  122,  122,  122,  122,  122,
 /*    10 */   122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
 /*    20 */   122,  191,  191,  191,  191,  191,  122,  122,  122,  122,
 /*    30 */   122,  122,  122,  316,  130,  191,  417,  417,   31,  332,
 /*    40 */   332,  332,  332,  131,  417,  417,  415,  417,  415,  415,
 /*    50 */   415,  415,  404,  417,  415,  404,  -20,  -20,  -20,  -19,
 /*    60 */    -5,    1,    1,    1,   32,   86,   86,   74,  388,  342,
 /*    70 */   360,  358,  346,  345,  338,  312,  324,  301,  294,  286,
 /*    80 */   284,  266,  271,  245,  269,  257,  246,  236,  254,  240,
 /*    90 */   227,  225,  194,  224,  212,  179,  187,  178,  176,  154,
 /*   100 */   162,  147,  141,  132,  113,   65,  110,   72,   45,   47,
 /*   110 */     7,   -2,
};
#define YY_REDUCE_USE_DFLT (-15)
#define YY_REDUCE_COUNT (58)
#define YY_REDUCE_MIN   (-14)
#define YY_REDUCE_MAX   (398)
static const short yy_reduce_ofst[] = {
 /*     0 */   177,  -14,   57,   39,   21,    4,  219,  205,  188,  174,
 /*    10 */   160,  146,  126,  112,   98,  233,  253,  243,  268,  321,
 /*    20 */   314,  320,  313,  306,  299,  292,  369,  362,  355,  348,
 /*    30 */   341,  333,  327,  -12,  398,  390,  149,   66,   29,  241,
 /*    40 */   307,   76,  208,   55,  100,   62,  392,  377,  389,  387,
 /*    50 */   386,  373,  364,  353,  359,  340,  347,  356,  361,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   175,  254,  206,  206,  206,  206,  254,  254,  254,  254,
 /*    10 */   254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
 /*    20 */   254,  202,  202,  202,  202,  202,  254,  254,  254,  254,
 /*    30 */   254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
 /*    40 */   254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
 /*    50 */   254,  254,  254,  254,  254,  254,  211,  195,  193,  248,
 /*    60 */   249,  247,  246,  245,  236,  254,  184,  254,  254,  254,
 /*    70 */   254,  254,  254,  225,  254,  224,  254,  254,  254,  254,
 /*    80 */   254,  254,  254,  213,  254,  254,  254,  254,  254,  254,
 /*    90 */   254,  254,  250,  254,  254,  182,  254,  254,  254,  207,
 /*   100 */   254,  184,  254,  254,  254,  254,  254,  203,  254,  254,
 /*   110 */   191,  184,  196,  200,  222,  221,  220,  219,  217,  216,
 /*   120 */   215,  214,  212,  210,  218,  229,  235,  234,  253,  252,
 /*   130 */   251,  244,  243,  242,  241,  240,  239,  238,  237,  233,
 /*   140 */   232,  231,  230,  208,  228,  227,  226,  225,  224,  223,
 /*   150 */   209,  199,  179,  194,  198,  183,  204,  205,  197,  201,
 /*   160 */   192,  182,  181,  190,  189,  188,  187,  186,  185,  184,
 /*   170 */   180,  178,  177,  176,
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
  "$",             "CLASS",         "LCURLYBRACKET",  "RCURLYBRACKET",
  "LROUNDBRACKET",  "RROUNDBRACKET",  "UCNAME",        "LCNAME",      
  "VOIDNAME",      "BOOLNAME",      "INTNAME",       "FLOATNAME",   
  "CONST",         "STAR",          "SEMICOLON",     "COMMA",       
  "IF",            "ELSE",          "FOR",           "COLON",       
  "WHILE",         "AUTO",          "SETEQUAL",      "AT",          
  "RETURN",        "BOOLCONSTANT",  "INTCONSTANT",   "FLOATCONSTANT",
  "DOT",           "PTRACCESS",     "PLUS",          "MINUS",       
  "EXCLAIMMARK",   "AMPERSAND",     "FORWARDSLASH",  "ISEQUAL",     
  "QUESTIONMARK",  "error",         "file",          "classDecl",   
  "classDef",      "classMethodDeclList",  "classMethodDecl",  "classMethodDefList",
  "classMethodDef",  "type",          "varName",       "methodName",  
  "typeName",      "typeVar",       "nonEmptyTypeVarList",  "typeVarList", 
  "value",         "nonEmptyValueList",  "valueList",     "scope",       
  "statementList",  "statement",     "precision0",    "precision1",  
  "precision2",    "precision3",    "precision4",    "precision5",  
  "precision6",    "precision7",    "start",         "ucName",      
  "lcName",      
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "start ::= file",
 /*   1 */ "file ::=",
 /*   2 */ "file ::= file classDecl",
 /*   3 */ "file ::= file classDef",
 /*   4 */ "classDecl ::= CLASS ucName LCURLYBRACKET classMethodDeclList RCURLYBRACKET",
 /*   5 */ "classDef ::= CLASS ucName LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classMethodDefList RCURLYBRACKET",
 /*   6 */ "ucName ::= UCNAME",
 /*   7 */ "lcName ::= LCNAME",
 /*   8 */ "varName ::= lcName",
 /*   9 */ "methodName ::= lcName",
 /*  10 */ "typeName ::= ucName",
 /*  11 */ "typeName ::= VOIDNAME",
 /*  12 */ "typeName ::= BOOLNAME",
 /*  13 */ "typeName ::= INTNAME",
 /*  14 */ "typeName ::= FLOATNAME",
 /*  15 */ "type ::= typeName",
 /*  16 */ "type ::= CONST typeName",
 /*  17 */ "type ::= type STAR",
 /*  18 */ "type ::= type STAR CONST",
 /*  19 */ "classMethodDeclList ::=",
 /*  20 */ "classMethodDeclList ::= classMethodDeclList classMethodDecl",
 /*  21 */ "classMethodDefList ::=",
 /*  22 */ "classMethodDefList ::= classMethodDefList classMethodDef",
 /*  23 */ "classMethodDecl ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON",
 /*  24 */ "classMethodDecl ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON",
 /*  25 */ "classMethodDef ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET scope",
 /*  26 */ "classMethodDef ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET scope",
 /*  27 */ "typeVar ::= type varName",
 /*  28 */ "typeVarList ::=",
 /*  29 */ "typeVarList ::= nonEmptyTypeVarList",
 /*  30 */ "nonEmptyTypeVarList ::= typeVar",
 /*  31 */ "nonEmptyTypeVarList ::= nonEmptyTypeVarList COMMA typeVar",
 /*  32 */ "valueList ::=",
 /*  33 */ "valueList ::= nonEmptyValueList",
 /*  34 */ "nonEmptyValueList ::= value",
 /*  35 */ "nonEmptyValueList ::= nonEmptyValueList COMMA value",
 /*  36 */ "scope ::= LCURLYBRACKET statementList RCURLYBRACKET",
 /*  37 */ "statementList ::=",
 /*  38 */ "statementList ::= statementList statement",
 /*  39 */ "statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope",
 /*  40 */ "statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope",
 /*  41 */ "statement ::= FOR LROUNDBRACKET varName COLON value RROUNDBRACKET scope",
 /*  42 */ "statement ::= WHILE LROUNDBRACKET value RROUNDBRACKET scope",
 /*  43 */ "statement ::= AUTO varName SETEQUAL value SEMICOLON",
 /*  44 */ "statement ::= type varName SETEQUAL value SEMICOLON",
 /*  45 */ "statement ::= lcName SETEQUAL value SEMICOLON",
 /*  46 */ "statement ::= AT lcName SETEQUAL value SEMICOLON",
 /*  47 */ "statement ::= value SEMICOLON",
 /*  48 */ "statement ::= RETURN value SEMICOLON",
 /*  49 */ "precision7 ::= LROUNDBRACKET precision0 RROUNDBRACKET",
 /*  50 */ "precision7 ::= lcName",
 /*  51 */ "precision7 ::= AT lcName",
 /*  52 */ "precision7 ::= BOOLCONSTANT",
 /*  53 */ "precision7 ::= INTCONSTANT",
 /*  54 */ "precision7 ::= FLOATCONSTANT",
 /*  55 */ "precision7 ::= ucName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  56 */ "precision7 ::= ucName COLON ucName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  57 */ "precision6 ::= precision7",
 /*  58 */ "precision6 ::= precision6 DOT varName",
 /*  59 */ "precision6 ::= precision6 DOT methodName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  60 */ "precision6 ::= precision6 PTRACCESS varName",
 /*  61 */ "precision6 ::= precision6 PTRACCESS methodName LROUNDBRACKET valueList RROUNDBRACKET",
 /*  62 */ "precision5 ::= precision6",
 /*  63 */ "precision5 ::= PLUS precision5",
 /*  64 */ "precision5 ::= MINUS precision5",
 /*  65 */ "precision5 ::= EXCLAIMMARK precision5",
 /*  66 */ "precision5 ::= AMPERSAND precision5",
 /*  67 */ "precision5 ::= STAR precision5",
 /*  68 */ "precision4 ::= precision5",
 /*  69 */ "precision4 ::= precision4 STAR precision5",
 /*  70 */ "precision4 ::= precision4 FORWARDSLASH precision5",
 /*  71 */ "precision3 ::= precision4",
 /*  72 */ "precision3 ::= precision3 PLUS precision4",
 /*  73 */ "precision3 ::= precision3 MINUS precision4",
 /*  74 */ "precision2 ::= precision3",
 /*  75 */ "precision2 ::= precision3 ISEQUAL precision3",
 /*  76 */ "precision1 ::= precision2",
 /*  77 */ "precision1 ::= precision2 QUESTIONMARK precision1 COLON precision1",
 /*  78 */ "precision0 ::= precision1",
 /*  79 */ "value ::= precision0",
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
  { 66, 1 },
  { 38, 0 },
  { 38, 2 },
  { 38, 2 },
  { 39, 5 },
  { 40, 8 },
  { 67, 1 },
  { 68, 1 },
  { 46, 1 },
  { 47, 1 },
  { 48, 1 },
  { 48, 1 },
  { 48, 1 },
  { 48, 1 },
  { 48, 1 },
  { 45, 1 },
  { 45, 2 },
  { 45, 2 },
  { 45, 3 },
  { 41, 0 },
  { 41, 2 },
  { 43, 0 },
  { 43, 2 },
  { 42, 5 },
  { 42, 6 },
  { 44, 5 },
  { 44, 6 },
  { 49, 2 },
  { 51, 0 },
  { 51, 1 },
  { 50, 1 },
  { 50, 3 },
  { 54, 0 },
  { 54, 1 },
  { 53, 1 },
  { 53, 3 },
  { 55, 3 },
  { 56, 0 },
  { 56, 2 },
  { 57, 5 },
  { 57, 7 },
  { 57, 7 },
  { 57, 5 },
  { 57, 5 },
  { 57, 5 },
  { 57, 4 },
  { 57, 5 },
  { 57, 2 },
  { 57, 3 },
  { 65, 3 },
  { 65, 1 },
  { 65, 2 },
  { 65, 1 },
  { 65, 1 },
  { 65, 1 },
  { 65, 4 },
  { 65, 6 },
  { 64, 1 },
  { 64, 3 },
  { 64, 6 },
  { 64, 3 },
  { 64, 6 },
  { 63, 1 },
  { 63, 2 },
  { 63, 2 },
  { 63, 2 },
  { 63, 2 },
  { 63, 2 },
  { 62, 1 },
  { 62, 3 },
  { 62, 3 },
  { 61, 1 },
  { 61, 3 },
  { 61, 3 },
  { 60, 1 },
  { 60, 3 },
  { 59, 1 },
  { 59, 5 },
  { 58, 1 },
  { 52, 1 },
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
#line 48 "Parser.y"
{
		printf("Parsed file successfully");
	}
#line 956 "Parser.c"
        break;
      case 1: /* file ::= */
#line 53 "Parser.y"
{
		yygotominor.yy70 = AST_MakeFile();
	}
#line 963 "Parser.c"
        break;
      case 2: /* file ::= file classDecl */
#line 58 "Parser.y"
{
		yygotominor.yy70 = AST_FileAddClassDecl(yymsp[-1].minor.yy70, yymsp[0].minor.yy108);
	}
#line 970 "Parser.c"
        break;
      case 3: /* file ::= file classDef */
#line 63 "Parser.y"
{
		yygotominor.yy70 = AST_FileAddClassDef(yymsp[-1].minor.yy70, yymsp[0].minor.yy15);
	}
#line 977 "Parser.c"
        break;
      case 4: /* classDecl ::= CLASS ucName LCURLYBRACKET classMethodDeclList RCURLYBRACKET */
#line 68 "Parser.y"
{
		yygotominor.yy108 = AST_MakeClassDecl(yymsp[-3].minor.yy0, yymsp[-1].minor.yy82);
	}
#line 984 "Parser.c"
        break;
      case 5: /* classDef ::= CLASS ucName LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classMethodDefList RCURLYBRACKET */
#line 73 "Parser.y"
{
		yygotominor.yy15 = AST_MakeClassDef(yymsp[-6].minor.yy0, yymsp[-4].minor.yy82, yymsp[-1].minor.yy82);
	}
#line 991 "Parser.c"
        break;
      case 6: /* ucName ::= UCNAME */
      case 7: /* lcName ::= LCNAME */ yytestcase(yyruleno==7);
#line 78 "Parser.y"
{
		yygotominor.yy0 = (yymsp[0].minor.yy0).str;
	}
#line 999 "Parser.c"
        break;
      case 8: /* varName ::= lcName */
      case 9: /* methodName ::= lcName */ yytestcase(yyruleno==9);
      case 10: /* typeName ::= ucName */ yytestcase(yyruleno==10);
#line 88 "Parser.y"
{
		yygotominor.yy91 = yymsp[0].minor.yy0;
	}
#line 1008 "Parser.c"
        break;
      case 11: /* typeName ::= VOIDNAME */
      case 12: /* typeName ::= BOOLNAME */ yytestcase(yyruleno==12);
      case 13: /* typeName ::= INTNAME */ yytestcase(yyruleno==13);
      case 14: /* typeName ::= FLOATNAME */ yytestcase(yyruleno==14);
#line 103 "Parser.y"
{
		yygotominor.yy91 = (yymsp[0].minor.yy0).str;
	}
#line 1018 "Parser.c"
        break;
      case 15: /* type ::= typeName */
#line 123 "Parser.y"
{
		yygotominor.yy104 = AST_MakeNamedType(AST_TYPE_MUTABLE, yymsp[0].minor.yy91);
	}
#line 1025 "Parser.c"
        break;
      case 16: /* type ::= CONST typeName */
#line 128 "Parser.y"
{
		yygotominor.yy104 = AST_MakeNamedType(AST_TYPE_CONST, yymsp[0].minor.yy91);
	}
#line 1032 "Parser.c"
        break;
      case 17: /* type ::= type STAR */
#line 133 "Parser.y"
{
		yygotominor.yy104 = AST_MakePtrType(AST_TYPE_MUTABLE, yymsp[-1].minor.yy104);
	}
#line 1039 "Parser.c"
        break;
      case 18: /* type ::= type STAR CONST */
#line 138 "Parser.y"
{
		yygotominor.yy104 = AST_MakePtrType(AST_TYPE_CONST, yymsp[-2].minor.yy104);
	}
#line 1046 "Parser.c"
        break;
      case 19: /* classMethodDeclList ::= */
      case 21: /* classMethodDefList ::= */ yytestcase(yyruleno==21);
      case 28: /* typeVarList ::= */ yytestcase(yyruleno==28);
      case 32: /* valueList ::= */ yytestcase(yyruleno==32);
      case 37: /* statementList ::= */ yytestcase(yyruleno==37);
#line 143 "Parser.y"
{
		yygotominor.yy82 = AST_ListCreate();
	}
#line 1057 "Parser.c"
        break;
      case 20: /* classMethodDeclList ::= classMethodDeclList classMethodDecl */
#line 148 "Parser.y"
{
		yygotominor.yy82 = AST_ListAppend(yymsp[-1].minor.yy82, yymsp[0].minor.yy131);
	}
#line 1064 "Parser.c"
        break;
      case 22: /* classMethodDefList ::= classMethodDefList classMethodDef */
#line 158 "Parser.y"
{
		yygotominor.yy82 = AST_ListAppend(yymsp[-1].minor.yy82, yymsp[0].minor.yy134);
	}
#line 1071 "Parser.c"
        break;
      case 23: /* classMethodDecl ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON */
#line 163 "Parser.y"
{
		yygotominor.yy131 = AST_MakeClassMethodDecl(0, yymsp[-4].minor.yy0, yymsp[-2].minor.yy82);
	}
#line 1078 "Parser.c"
        break;
      case 24: /* classMethodDecl ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON */
#line 168 "Parser.y"
{
		yygotominor.yy131 = AST_MakeClassMethodDecl(yymsp[-5].minor.yy104, yymsp[-4].minor.yy91, yymsp[-2].minor.yy82);
	}
#line 1085 "Parser.c"
        break;
      case 25: /* classMethodDef ::= ucName LROUNDBRACKET typeVarList RROUNDBRACKET scope */
#line 173 "Parser.y"
{
		yygotominor.yy134 = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(0, yymsp[-4].minor.yy0, yymsp[-2].minor.yy82), yymsp[0].minor.yy26);
	}
#line 1092 "Parser.c"
        break;
      case 26: /* classMethodDef ::= type methodName LROUNDBRACKET typeVarList RROUNDBRACKET scope */
#line 178 "Parser.y"
{
		yygotominor.yy134 = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(yymsp[-5].minor.yy104, yymsp[-4].minor.yy91, yymsp[-2].minor.yy82), yymsp[0].minor.yy26);
	}
#line 1099 "Parser.c"
        break;
      case 27: /* typeVar ::= type varName */
#line 183 "Parser.y"
{
		yygotominor.yy29 = AST_MakeTypeVar(yymsp[-1].minor.yy104, yymsp[0].minor.yy91);
	}
#line 1106 "Parser.c"
        break;
      case 29: /* typeVarList ::= nonEmptyTypeVarList */
      case 33: /* valueList ::= nonEmptyValueList */ yytestcase(yyruleno==33);
#line 193 "Parser.y"
{
		yygotominor.yy82 = yymsp[0].minor.yy82;
	}
#line 1114 "Parser.c"
        break;
      case 30: /* nonEmptyTypeVarList ::= typeVar */
#line 198 "Parser.y"
{
		yygotominor.yy82 = AST_ListAppend(AST_ListCreate(), yymsp[0].minor.yy29);
	}
#line 1121 "Parser.c"
        break;
      case 31: /* nonEmptyTypeVarList ::= nonEmptyTypeVarList COMMA typeVar */
#line 203 "Parser.y"
{
		yygotominor.yy82 = AST_ListAppend(yymsp[-2].minor.yy82, yymsp[0].minor.yy29);
	}
#line 1128 "Parser.c"
        break;
      case 34: /* nonEmptyValueList ::= value */
#line 218 "Parser.y"
{
		yygotominor.yy82 = AST_ListAppend(AST_ListCreate(), yymsp[0].minor.yy17);
	}
#line 1135 "Parser.c"
        break;
      case 35: /* nonEmptyValueList ::= nonEmptyValueList COMMA value */
#line 223 "Parser.y"
{
		yygotominor.yy82 = AST_ListAppend(yymsp[-2].minor.yy82, yymsp[0].minor.yy17);
	}
#line 1142 "Parser.c"
        break;
      case 36: /* scope ::= LCURLYBRACKET statementList RCURLYBRACKET */
#line 228 "Parser.y"
{
		yygotominor.yy26 = AST_MakeScope(yymsp[-1].minor.yy82);
	}
#line 1149 "Parser.c"
        break;
      case 38: /* statementList ::= statementList statement */
#line 238 "Parser.y"
{
		yygotominor.yy82 = AST_ListAppend(yymsp[-1].minor.yy82, yymsp[0].minor.yy99);
	}
#line 1156 "Parser.c"
        break;
      case 39: /* statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope */
#line 243 "Parser.y"
{
		yygotominor.yy99 = AST_MakeIf(yymsp[-2].minor.yy17, yymsp[0].minor.yy26, NULL);
	}
#line 1163 "Parser.c"
        break;
      case 40: /* statement ::= IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope */
#line 248 "Parser.y"
{
		yygotominor.yy99 = AST_MakeIf(yymsp[-4].minor.yy17, yymsp[-2].minor.yy26, yymsp[0].minor.yy26);
	}
#line 1170 "Parser.c"
        break;
      case 41: /* statement ::= FOR LROUNDBRACKET varName COLON value RROUNDBRACKET scope */
      case 42: /* statement ::= WHILE LROUNDBRACKET value RROUNDBRACKET scope */ yytestcase(yyruleno==42);
#line 253 "Parser.y"
{
		// TODO
		yygotominor.yy99 = yymsp[-2].minor.yy17;
	}
#line 1179 "Parser.c"
        break;
      case 43: /* statement ::= AUTO varName SETEQUAL value SEMICOLON */
#line 265 "Parser.y"
{
		yygotominor.yy99 = AST_MakeAutoVarDecl(yymsp[-3].minor.yy91, yymsp[-1].minor.yy17);
	}
#line 1186 "Parser.c"
        break;
      case 44: /* statement ::= type varName SETEQUAL value SEMICOLON */
#line 270 "Parser.y"
{
		yygotominor.yy99 = AST_MakeVarDecl(yymsp[-4].minor.yy104, yymsp[-3].minor.yy91, yymsp[-1].minor.yy17);
	}
#line 1193 "Parser.c"
        break;
      case 45: /* statement ::= lcName SETEQUAL value SEMICOLON */
#line 275 "Parser.y"
{
		yygotominor.yy99 = AST_MakeVarAssign(AST_MakeLocalVar(yymsp[-3].minor.yy0), yymsp[-1].minor.yy17);
	}
#line 1200 "Parser.c"
        break;
      case 46: /* statement ::= AT lcName SETEQUAL value SEMICOLON */
#line 280 "Parser.y"
{
		yygotominor.yy99 = AST_MakeVarAssign(AST_MakeThisVar(yymsp[-3].minor.yy0), yymsp[-1].minor.yy17);
	}
#line 1207 "Parser.c"
        break;
      case 47: /* statement ::= value SEMICOLON */
#line 285 "Parser.y"
{
		yygotominor.yy99 = AST_MakeValueStmt(yymsp[-1].minor.yy17);
	}
#line 1214 "Parser.c"
        break;
      case 48: /* statement ::= RETURN value SEMICOLON */
#line 290 "Parser.y"
{
		yygotominor.yy99 = AST_MakeReturn(yymsp[-1].minor.yy17);
	}
#line 1221 "Parser.c"
        break;
      case 49: /* precision7 ::= LROUNDBRACKET precision0 RROUNDBRACKET */
#line 295 "Parser.y"
{
		yygotominor.yy17 = yymsp[-1].minor.yy17;
	}
#line 1228 "Parser.c"
        break;
      case 50: /* precision7 ::= lcName */
#line 300 "Parser.y"
{
		yygotominor.yy17 = AST_MakeVarAccess(AST_MakeLocalVar(yymsp[0].minor.yy0));
	}
#line 1235 "Parser.c"
        break;
      case 51: /* precision7 ::= AT lcName */
#line 305 "Parser.y"
{
		yygotominor.yy17 = AST_MakeVarAccess(AST_MakeThisVar(yymsp[0].minor.yy0));
	}
#line 1242 "Parser.c"
        break;
      case 52: /* precision7 ::= BOOLCONSTANT */
#line 310 "Parser.y"
{
		yygotominor.yy17 = AST_MakeBoolConstant((yymsp[0].minor.yy0).boolValue);
	}
#line 1249 "Parser.c"
        break;
      case 53: /* precision7 ::= INTCONSTANT */
#line 315 "Parser.y"
{
		yygotominor.yy17 = AST_MakeIntConstant((yymsp[0].minor.yy0).intValue);
	}
#line 1256 "Parser.c"
        break;
      case 54: /* precision7 ::= FLOATCONSTANT */
#line 320 "Parser.y"
{
		yygotominor.yy17 = AST_MakeFloatConstant((yymsp[0].minor.yy0).floatValue);
	}
#line 1263 "Parser.c"
        break;
      case 55: /* precision7 ::= ucName LROUNDBRACKET valueList RROUNDBRACKET */
#line 325 "Parser.y"
{
		yygotominor.yy17 = AST_MakeConstruct(yymsp[-3].minor.yy0, NULL, yymsp[-1].minor.yy82);
	}
#line 1270 "Parser.c"
        break;
      case 56: /* precision7 ::= ucName COLON ucName LROUNDBRACKET valueList RROUNDBRACKET */
#line 330 "Parser.y"
{
		yygotominor.yy17 = AST_MakeConstruct(yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy82);
	}
#line 1277 "Parser.c"
        break;
      case 57: /* precision6 ::= precision7 */
      case 62: /* precision5 ::= precision6 */ yytestcase(yyruleno==62);
      case 68: /* precision4 ::= precision5 */ yytestcase(yyruleno==68);
      case 71: /* precision3 ::= precision4 */ yytestcase(yyruleno==71);
      case 74: /* precision2 ::= precision3 */ yytestcase(yyruleno==74);
      case 76: /* precision1 ::= precision2 */ yytestcase(yyruleno==76);
      case 78: /* precision0 ::= precision1 */ yytestcase(yyruleno==78);
      case 79: /* value ::= precision0 */ yytestcase(yyruleno==79);
#line 335 "Parser.y"
{
		yygotominor.yy17 = yymsp[0].minor.yy17;
	}
#line 1291 "Parser.c"
        break;
      case 58: /* precision6 ::= precision6 DOT varName */
#line 340 "Parser.y"
{
		yygotominor.yy17 = AST_MakeMemberAccess(yymsp[-2].minor.yy17, yymsp[0].minor.yy91);
	}
#line 1298 "Parser.c"
        break;
      case 59: /* precision6 ::= precision6 DOT methodName LROUNDBRACKET valueList RROUNDBRACKET */
#line 345 "Parser.y"
{
		yygotominor.yy17 = AST_MakeMethodCall(yymsp[-5].minor.yy17, yymsp[-3].minor.yy91, yymsp[-1].minor.yy82);
	}
#line 1305 "Parser.c"
        break;
      case 60: /* precision6 ::= precision6 PTRACCESS varName */
#line 350 "Parser.y"
{
		yygotominor.yy17 = AST_MakeMemberAccess(AST_MakeUnary(AST_UNARY_DEREF, yymsp[-2].minor.yy17), yymsp[0].minor.yy91);
	}
#line 1312 "Parser.c"
        break;
      case 61: /* precision6 ::= precision6 PTRACCESS methodName LROUNDBRACKET valueList RROUNDBRACKET */
#line 355 "Parser.y"
{
		yygotominor.yy17 = AST_MakeMethodCall(AST_MakeUnary(AST_UNARY_DEREF, yymsp[-5].minor.yy17), yymsp[-3].minor.yy91, yymsp[-1].minor.yy82);
	}
#line 1319 "Parser.c"
        break;
      case 63: /* precision5 ::= PLUS precision5 */
#line 365 "Parser.y"
{
		yygotominor.yy17 = AST_MakeUnary(AST_UNARY_PLUS, yymsp[0].minor.yy17);
	}
#line 1326 "Parser.c"
        break;
      case 64: /* precision5 ::= MINUS precision5 */
#line 370 "Parser.y"
{
		yygotominor.yy17 = AST_MakeUnary(AST_UNARY_MINUS, yymsp[0].minor.yy17);
	}
#line 1333 "Parser.c"
        break;
      case 65: /* precision5 ::= EXCLAIMMARK precision5 */
#line 375 "Parser.y"
{
		yygotominor.yy17 = AST_MakeUnary(AST_UNARY_NEGATE, yymsp[0].minor.yy17);
	}
#line 1340 "Parser.c"
        break;
      case 66: /* precision5 ::= AMPERSAND precision5 */
#line 380 "Parser.y"
{
		yygotominor.yy17 = AST_MakeUnary(AST_UNARY_ADDRESSOF, yymsp[0].minor.yy17);
	}
#line 1347 "Parser.c"
        break;
      case 67: /* precision5 ::= STAR precision5 */
#line 385 "Parser.y"
{
		yygotominor.yy17 = AST_MakeUnary(AST_UNARY_DEREF, yymsp[0].minor.yy17);
	}
#line 1354 "Parser.c"
        break;
      case 69: /* precision4 ::= precision4 STAR precision5 */
#line 395 "Parser.y"
{
		yygotominor.yy17 = AST_MakeBinary(AST_BINARY_MULTIPLY, yymsp[-2].minor.yy17, yymsp[0].minor.yy17);
	}
#line 1361 "Parser.c"
        break;
      case 70: /* precision4 ::= precision4 FORWARDSLASH precision5 */
#line 400 "Parser.y"
{
		yygotominor.yy17 = AST_MakeBinary(AST_BINARY_DIVIDE, yymsp[-2].minor.yy17, yymsp[0].minor.yy17);
	}
#line 1368 "Parser.c"
        break;
      case 72: /* precision3 ::= precision3 PLUS precision4 */
#line 410 "Parser.y"
{
		yygotominor.yy17 = AST_MakeBinary(AST_BINARY_ADD, yymsp[-2].minor.yy17, yymsp[0].minor.yy17);
	}
#line 1375 "Parser.c"
        break;
      case 73: /* precision3 ::= precision3 MINUS precision4 */
#line 415 "Parser.y"
{
		yygotominor.yy17 = AST_MakeBinary(AST_BINARY_SUBTRACT, yymsp[-2].minor.yy17, yymsp[0].minor.yy17);
	}
#line 1382 "Parser.c"
        break;
      case 75: /* precision2 ::= precision3 ISEQUAL precision3 */
#line 425 "Parser.y"
{
		yygotominor.yy17 = AST_MakeBinary(AST_BINARY_ISEQUAL, yymsp[-2].minor.yy17, yymsp[0].minor.yy17);
	}
#line 1389 "Parser.c"
        break;
      case 77: /* precision1 ::= precision2 QUESTIONMARK precision1 COLON precision1 */
#line 435 "Parser.y"
{
		yygotominor.yy17 = AST_MakeTernary(yymsp[-4].minor.yy17, yymsp[-2].minor.yy17, yymsp[0].minor.yy17);
	}
#line 1396 "Parser.c"
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
