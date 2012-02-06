%include {#include <assert.h>}
%include {#include <stdio.h>}
%include {#include <string.h>}
%include {#include <Locic/AST.h>}
%include {#include <Locic/List.h>}
%include {#include <Locic/ParserContext.h>}
%include {#include <Locic/Token.h>}

%name Locic_Parse
%extra_argument { Locic_ParserContext * parserContext }
%start_symbol start
%token_prefix LOCIC_TOKEN_
%token_type { Locic_Token }

%parse_accept {
	//printf("Success!\n");
}

%parse_failure {
	//printf("Failure!\n");
	parserContext->parseFailed = 1;
}

%syntax_error {
	printf("Syntax error on line %d\n", (int) parserContext->lineNumber);
}

%type module { AST_Module * }

%type classDecl { AST_ClassDecl * }
%type classDef { AST_ClassDef * }

%type functionDecl { AST_FunctionDecl * }
%type functionDef { AST_FunctionDef * }

%type classMethodDeclList { Locic_List * }
%type classMethodDefList { Locic_List * }

%type basicType { AST_BasicTypeEnum }
%type type { AST_Type * }
%type nonEmptyTypeList { Locic_List * }
%type typeList { Locic_List * }

%type lcName { char * }
%type ucName { char * }
%type typeName { char * }

%type typeVar { AST_TypeVar * }
%type nonEmptyTypeVarList { Locic_List * }
%type typeVarList { Locic_List * }

%type value { AST_Value * }
%type nonEmptyValueList { Locic_List * }
%type valueList { Locic_List * }

%type scope { AST_Scope * }
%type statementList { Locic_List * }
%type statement { AST_Statement * }

%type precision0 { AST_Value * }
%type precision1 { AST_Value * }
%type precision2 { AST_Value * }
%type precision3 { AST_Value * }
%type precision4 { AST_Value * }
%type precision5 { AST_Value * }
%type precision6 { AST_Value * }
%type precision7 { AST_Value * }
	
start ::= module(M) .
	{
		printf("Completed parsing\n");
		Locic_List_Append(parserContext->moduleGroup->modules, M);
	}
	
// Nasty hack to create ERROR token and error non-terminal (UNKNOWN can never be sent by the lexer).
start ::= UNKNOWN ERROR error.

module(M) ::= .
	{
		M = AST_MakeModule(strcpy(malloc(strlen(parserContext->currentFileName) + 1), parserContext->currentFileName));
	}
	
module(M) ::= INTERFACE.
	{
		M = AST_MakeModule(strcpy(malloc(strlen(parserContext->currentFileName) + 1), parserContext->currentFileName));
	}
	
module(NM) ::= module(OM) functionDecl(D).
	{
		NM = AST_ModuleAddFunctionDecl(OM, D);
	}

module(NM) ::= module(OM) functionDef(D).
	{
		NM = AST_ModuleAddFunctionDef(OM, D);
	}

module(NM) ::= module(OM) classDecl(D).
	{
		NM = AST_ModuleAddClassDecl(OM, D);
	}

module(NM) ::= module(OM) classDef(D).
	{
		NM = AST_ModuleAddClassDef(OM, D);
	}
	
functionDecl(D) ::= type(T) lcName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET SEMICOLON.
	{
		D = AST_MakeFunctionDecl(T, N, P);
	}
	
functionDef(D) ::= type(T) lcName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET scope(S).
	{
		D = AST_MakeFunctionDef(AST_MakeFunctionDecl(T, N, P), S);
	}
	
classDecl(D) ::= CLASS ucName(N) LCURLYBRACKET classMethodDeclList(DL) RCURLYBRACKET.
	{
		D = AST_MakeClassDecl(N, DL);
	}
	
classDef(D) ::= CLASS ucName(N) LROUNDBRACKET typeVarList(VL) RROUNDBRACKET LCURLYBRACKET classMethodDefList(DL) RCURLYBRACKET.
	{
		D = AST_MakeClassDef(N, VL, DL);
	}
	
lcName(N) ::= LCNAME(NAME).
	{
		N = (NAME).str;
	}
	
ucName(N) ::= UCNAME(NAME).
	{
		N = (NAME).str;
	}
	
basicType(T) ::= VOIDNAME.
	{
		T = AST_TYPE_BASIC_VOID;
	}
	
basicType(T) ::= BOOLNAME.
	{
		T = AST_TYPE_BASIC_BOOL;
	}
	
basicType(T) ::= INTNAME.
	{
		T = AST_TYPE_BASIC_INT;
	}
	
basicType(T) ::= FLOATNAME.
	{
		T = AST_TYPE_BASIC_FLOAT;
	}
	
type(T) ::= basicType(BT).
	{
		T = AST_MakeBasicType(AST_TYPE_MUTABLE, BT);
	}

type(T) ::= CONST basicType(BT).
	{
		T = AST_MakeBasicType(AST_TYPE_CONST, BT);
	}
	
type(T) ::= ucName(N).
	{
		T = AST_MakeNamedType(AST_TYPE_MUTABLE, N);
	}

type(T) ::= CONST ucName(N).
	{
		T = AST_MakeNamedType(AST_TYPE_CONST, N);
	}

type(NT) ::= type(OT) STAR.
	{
		NT = AST_MakePtrType(AST_TYPE_MUTABLE, OT);
	}

type(NT) ::= type(OT) STAR CONST.
	{
		NT = AST_MakePtrType(AST_TYPE_CONST, OT);
	}
	
type(NT) ::= LROUNDBRACKET type(RT) RROUNDBRACKET LROUNDBRACKET typeList(PTL) RROUNDBRACKET.
	{
		NT = AST_MakeFuncType(AST_TYPE_MUTABLE, RT, PTL);
	}
	
type(NT) ::= LROUNDBRACKET type(RT) RROUNDBRACKET LROUNDBRACKET typeList(PTL) RROUNDBRACKET CONST.
	{
		NT = AST_MakeFuncType(AST_TYPE_CONST, RT, PTL);
	}
	
nonEmptyTypeList(TL) ::= type(T).
	{
		TL = Locic_List_Append(Locic_List_Alloc(), T);
	}
	
nonEmptyTypeList(TL) ::= nonEmptyTypeList(OTL) COMMA type(T).
	{
		TL = Locic_List_Append(OTL, T);
	}
	
typeList(TL) ::= .
	{
		TL = Locic_List_Alloc();
	}
	
typeList(TL) ::= nonEmptyTypeList(NETL).
	{
		TL = NETL;
	}
	
classMethodDeclList(DL) ::= .
	{
		DL = Locic_List_Alloc();
	}
	
classMethodDeclList(DL) ::= classMethodDeclList(ODL) functionDecl(D).
	{
		DL = Locic_List_Append(ODL, D);
	}
	
classMethodDefList(DL) ::= .
	{
		DL = Locic_List_Alloc();
	}
	
classMethodDefList(DL) ::= classMethodDefList(ODL) functionDef(D).
	{
		DL = Locic_List_Append(ODL, D);
	}
	
typeVar(TV) ::= type(T) lcName(N).
	{
		TV = AST_MakeTypeVar(T, N);
	}
	
typeVarList(TVL) ::= .
	{
		TVL = Locic_List_Alloc();
	}
	
typeVarList(TVL) ::= nonEmptyTypeVarList(L).
	{
		TVL = L;
	}
	
nonEmptyTypeVarList(TVL) ::= typeVar(TV).
	{
		TVL = Locic_List_Append(Locic_List_Alloc(), TV);
	}
	
nonEmptyTypeVarList(TVL) ::= nonEmptyTypeVarList(L) COMMA typeVar(TV).
	{
		TVL = Locic_List_Append(L, TV);
	}
	
valueList(VL) ::= .
	{
		VL = Locic_List_Alloc();
	}
	
valueList(VL) ::= nonEmptyValueList(L).
	{
		VL = L;
	}
	
nonEmptyValueList(VL) ::= value(V).
	{
		VL = Locic_List_Append(Locic_List_Alloc(), V);
	}
	
nonEmptyValueList(VL) ::= nonEmptyValueList(L) COMMA value(V).
	{
		VL = Locic_List_Append(L, V);
	}
	
scope(S) ::= LCURLYBRACKET statementList(SL) RCURLYBRACKET.
	{
		S = AST_MakeScope(SL);
	}
	
statementList(SL) ::= .
	{
		SL = Locic_List_Alloc();
	}
	
statementList(SL) ::= statementList(L) statement(S).
	{
		SL = Locic_List_Append(L, S);
	}
	
statement(S) ::= IF LROUNDBRACKET value(V) RROUNDBRACKET scope(T).
	{
		S = AST_MakeIf(V, T, NULL);
	}
	
statement(S) ::= IF LROUNDBRACKET value(V) RROUNDBRACKET scope(T) ELSE scope(F).
	{
		S = AST_MakeIf(V, T, F);
	}
	
statement(S) ::= FOR LROUNDBRACKET type lcName COLON value(V) RROUNDBRACKET scope.
	{
		// TODO
		S = AST_MakeValueStmt(V);
	}
	
statement(S) ::= WHILE LROUNDBRACKET value(V) RROUNDBRACKET scope.
	{
		// TODO
		S = AST_MakeValueStmt(V);
	}
	
statement(S) ::= AUTO lcName(N) SETEQUAL value(V) SEMICOLON.
	{
		S = AST_MakeAutoVarDecl(N, V);
	}
	
statement(S) ::= type(T) lcName(N) SETEQUAL value(V) SEMICOLON.
	{
		S = AST_MakeVarDecl(T, N, V);
	}

statement(S) ::= value(LV) SETEQUAL value(RV) SEMICOLON.
	{
		S = AST_MakeAssign(LV, RV);
	}

statement(S) ::= value(V) SEMICOLON.
	{
		S = AST_MakeValueStmt(V);
	}

statement(S) ::= RETURN value(V) SEMICOLON.
	{
		S = AST_MakeReturn(V);
	}

precision7(V) ::= LROUNDBRACKET precision0(BV) RROUNDBRACKET.
	{
		V = BV;
	}

precision7(V) ::= lcName(N).
	{
		V = AST_MakeVarValue(AST_MakeLocalVar(N));
	}

precision7(V) ::= AT lcName(N).
	{
		V = AST_MakeVarValue(AST_MakeThisVar(N));
	}

precision7(V) ::= BOOLCONSTANT(C).
	{
		V = AST_MakeBoolConstant((C).boolValue);
	}
	
precision7(V) ::= INTCONSTANT(C).
	{
		V = AST_MakeIntConstant((C).intValue);
	}
	
precision7(V) ::= FLOATCONSTANT(C).
	{
		V = AST_MakeFloatConstant((C).floatValue);
	}

precision7(V) ::= ucName(N) LROUNDBRACKET valueList(VL) RROUNDBRACKET.
	{
		V = AST_MakeConstruct(N, NULL, VL);
	}

precision7(V) ::= ucName(TN) COLON ucName(CN) LROUNDBRACKET valueList(VL) RROUNDBRACKET.
	{
		V = AST_MakeConstruct(TN, CN, VL);
	}
	
precision7(V) ::= CAST LTRIBRACKET type(T) RTRIBRACKET LROUNDBRACKET value(VAL) RROUNDBRACKET.
	{
		V = AST_MakeCast(T, VAL);
	}

precision6(V) ::= precision7(VAL).
	{
		V = VAL;
	}

precision6(V) ::= precision6(S) DOT lcName(N).
	{
		V = AST_MakeMemberAccess(S, N);
	}

precision6(V) ::= precision6(SP) PTRACCESS lcName(N).
	{
		V = AST_MakeMemberAccess(AST_MakeUnary(AST_UNARY_DEREF, SP), N);
	}

precision6(V) ::= precision6(F) LROUNDBRACKET valueList(P) RROUNDBRACKET.
	{
		V = AST_MakeFunctionCall(F, P);
	}
	
precision5(V) ::= precision6(VAL).
	{
		V = VAL;
	}

precision5(V) ::= PLUS precision5(VAL).
	{
		V = AST_MakeUnary(AST_UNARY_PLUS, VAL);
	}

precision5(V) ::= MINUS precision5(VAL).
	{
		V = AST_MakeUnary(AST_UNARY_MINUS, VAL);
	}

precision5(V) ::= EXCLAIMMARK precision5(VAL).
	{
		V = AST_MakeUnary(AST_UNARY_NOT, VAL);
	}

precision5(V) ::= AMPERSAND precision5(VAL).
	{
		V = AST_MakeUnary(AST_UNARY_ADDRESSOF, VAL);
	}

precision5(V) ::= STAR precision5(VAL).
	{
		V = AST_MakeUnary(AST_UNARY_DEREF, VAL);
	}

precision4(V) ::= precision5(VAL).
	{
		V = VAL;
	}

precision4(V) ::= precision4(L) STAR precision5(R).
	{
		V = AST_MakeBinary(AST_BINARY_MULTIPLY, L, R);
	}

precision4(V) ::= precision4(L) FORWARDSLASH precision5(R).
	{
		V = AST_MakeBinary(AST_BINARY_DIVIDE, L, R);
	}

precision3(V) ::= precision4(VAL).
	{
		V = VAL;
	}

precision3(V) ::= precision3(L) PLUS precision4(R).
	{
		V = AST_MakeBinary(AST_BINARY_ADD, L, R);
	}

precision3(V) ::= precision3(L) MINUS precision4(R).
	{
		V = AST_MakeBinary(AST_BINARY_SUBTRACT, L, R);
	}

precision2(V) ::= precision3(VAL).
	{
		V = VAL;
	}

precision2(V) ::= precision3(L) ISEQUAL precision3(R).
	{
		V = AST_MakeBinary(AST_BINARY_ISEQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) NOTEQUAL precision3(R).
	{
		V = AST_MakeBinary(AST_BINARY_NOTEQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) GREATEROREQUAL precision3(R).
	{
		V = AST_MakeBinary(AST_BINARY_GREATEROREQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) LESSOREQUAL precision3(R).
	{
		V = AST_MakeBinary(AST_BINARY_LESSOREQUAL, L, R);
	}

precision1(V) ::= precision2(VAL).
	{
		V = VAL;
	}

precision1(V) ::= precision2(C) QUESTIONMARK precision1(T) COLON precision1(F).
	{
		V = AST_MakeTernary(C, T, F);
	}

precision0(V) ::= precision1(VAL).
	{
		V = VAL;
	}

value(V) ::= precision0(VAL).
	{
		V = VAL;
	}


