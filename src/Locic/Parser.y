%include {#include <assert.h>}
%include {#include <stdio.h>}
%include {#include <Locic/Token.h>}
%include {#include <Locic/AST.h>}

%name Locic_Parse
%start_symbol start
%token_prefix LOCIC_TOKEN_
%token_type { Locic_Token }
%extra_argument { AST_File ** resultAST }

%parse_accept {
	printf("Success!\n");
}

%parse_failure {
	printf("Failure!\n");
}

%syntax_error {
	printf("Syntax error\n");
}

%type start { AST_File * }
%type file { AST_File * }

%type classDecl { AST_ClassDecl * }
%type classDef { AST_ClassDef * }

%type classMethodDeclList { AST_List * }
%type classMethodDecl { AST_ClassMethodDecl * }
%type classMethodDefList { AST_List * }
%type classMethodDef { AST_ClassMethodDef * }

%type type { AST_Type * }

%type ucName { char * }
%type lcName { char * }

%type varName { char * }
%type methodName { char * }
%type typeName { char * }

%type typeVar { AST_TypeVar * }
%type nonEmptyTypeVarList { AST_List * }
%type typeVarList { AST_List * }

%type value { AST_Value * }
%type nonEmptyValueList { AST_List * }
%type valueList { AST_List * }

%type scope { AST_Scope * }
%type statementList { AST_List * }
%type statement { AST_Statement * }

%type precision0 { AST_Value * }
%type precision1 { AST_Value * }
%type precision2 { AST_Value * }
%type precision3 { AST_Value * }
%type precision4 { AST_Value * }
%type precision5 { AST_Value * }
%type precision6 { AST_Value * }
%type precision7 { AST_Value * }
	
start(S) ::= file(F) .
	{
		printf("Completed parsing\n");
		S = F;
	}

file(F) ::= .
	{
		F = AST_MakeFile();
	}

file(F) ::= ERROR.
	{
		F = AST_MakeFile();
	}
	
file(F) ::= error.
	{
		printf("ERROR\n");
		F = AST_MakeFile();
	}
	
file(F) ::= INTERFACE.
	{
		F = AST_MakeFile();
	}

file(NF) ::= file(OF) classDecl(D).
	{
		NF = AST_FileAddClassDecl(OF, D);
	}

file(NF) ::= file(OF) classDef(D).
	{
		NF = AST_FileAddClassDef(OF, D);
	}
	
classDecl(D) ::= CLASS ucName(N) LCURLYBRACKET classMethodDeclList(DL) RCURLYBRACKET.
	{
		D = AST_MakeClassDecl(N, DL);
	}
	
classDef(D) ::= CLASS ucName(N) LROUNDBRACKET typeVarList(VL) RROUNDBRACKET LCURLYBRACKET classMethodDefList(DL) RCURLYBRACKET.
	{
		D = AST_MakeClassDef(N, VL, DL);
	}
	
ucName(N) ::= UCNAME(NAME).
	{
		N = (NAME).value.str;
	}
	
lcName(N) ::= LCNAME(NAME).
	{
		N = (NAME).value.str;
	}
	
varName(V) ::= lcName(N).
	{
		V = N;
	}
	
methodName(M) ::= lcName(N).
	{
		M = N;
	}
	
typeName(T) ::= ucName(N).
	{
		T = N;
	}
	
typeName(T) ::= VOIDNAME(N).
	{
		T = (N).value.str;
	}
	
typeName(T) ::= BOOLNAME(N).
	{
		T = (N).value.str;
	}
	
typeName(T) ::= INTNAME(N).
	{
		T = (N).value.str;
	}
	
typeName(T) ::= FLOATNAME(N).
	{
		T = (N).value.str;
	}
	
type(T) ::= typeName(N).
	{
		T = AST_MakeNamedType(AST_TYPE_MUTABLE, N);
	}

type(T) ::= CONST typeName(TN).
	{
		T = AST_MakeNamedType(AST_TYPE_CONST, TN);
	}

type(NT) ::= type(OT) STAR.
	{
		NT = AST_MakePtrType(AST_TYPE_MUTABLE, OT);
	}

type(NT) ::= type(OT)STAR CONST.
	{
		NT = AST_MakePtrType(AST_TYPE_CONST, OT);
	}
	
classMethodDeclList(DL) ::= .
	{
		DL = AST_ListCreate();
	}
	
classMethodDeclList(DL) ::= classMethodDeclList(ODL) classMethodDecl(D).
	{
		DL = AST_ListAppend(ODL, D);
	}
	
classMethodDefList(DL) ::= .
	{
		DL = AST_ListCreate();
	}
	
classMethodDefList(DL) ::= classMethodDefList(ODL) classMethodDef(D).
	{
		DL = AST_ListAppend(ODL, D);
	}
	
classMethodDecl(D) ::= ucName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET SEMICOLON.
	{
		D = AST_MakeClassMethodDecl(0, N, P);
	}
	
classMethodDecl(D) ::= type(T) methodName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET SEMICOLON.
	{
		D = AST_MakeClassMethodDecl(T, N, P);
	}
	
classMethodDef(D) ::= ucName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET scope(S).
	{
		D = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(0, N, P), S);
	}
	
classMethodDef(D) ::= type(T) methodName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET scope(S).
	{
		D = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(T, N, P), S);
	}
	
typeVar(TV) ::= type(T) varName(N).
	{
		TV = AST_MakeTypeVar(T, N);
	}
	
typeVarList(TVL) ::= .
	{
		TVL = AST_ListCreate();
	}
	
typeVarList(TVL) ::= nonEmptyTypeVarList(L).
	{
		TVL = L;
	}
	
nonEmptyTypeVarList(TVL) ::= typeVar(TV).
	{
		TVL = AST_ListAppend(AST_ListCreate(), TV);
	}
	
nonEmptyTypeVarList(TVL) ::= nonEmptyTypeVarList(L) COMMA typeVar(TV).
	{
		TVL = AST_ListAppend(L, TV);
	}
	
valueList(VL) ::= .
	{
		VL = AST_ListCreate();
	}
	
valueList(VL) ::= nonEmptyValueList(L).
	{
		VL = L;
	}
	
nonEmptyValueList(VL) ::= value(V).
	{
		VL = AST_ListAppend(AST_ListCreate(), V);
	}
	
nonEmptyValueList(VL) ::= nonEmptyValueList(L) COMMA value(V).
	{
		VL = AST_ListAppend(L, V);
	}
	
scope(S) ::= LCURLYBRACKET statementList(SL) RCURLYBRACKET.
	{
		S = AST_MakeScope(SL);
	}
	
statementList(SL) ::= .
	{
		SL = AST_ListCreate();
	}
	
statementList(SL) ::= statementList(L) statement(S).
	{
		SL = AST_ListAppend(L, S);
	}
	
statement(S) ::= IF LROUNDBRACKET value(V) RROUNDBRACKET scope(T).
	{
		S = AST_MakeIf(V, T, NULL);
	}
	
statement(S) ::= IF LROUNDBRACKET value(V) RROUNDBRACKET scope(T) ELSE scope(F).
	{
		S = AST_MakeIf(V, T, F);
	}
	
statement(S) ::= FOR LROUNDBRACKET varName COLON value(V) RROUNDBRACKET scope.
	{
		// TODO
		S = AST_MakeValueStmt(V);
	}
	
statement(S) ::= WHILE LROUNDBRACKET value(V) RROUNDBRACKET scope.
	{
		// TODO
		S = AST_MakeValueStmt(V);
	}
	
statement(S) ::= AUTO varName(N) SETEQUAL value(V) SEMICOLON.
	{
		S = AST_MakeAutoVarDecl(N, V);
	}
	
statement(S) ::= type(T) varName(N) SETEQUAL value(V) SEMICOLON.
	{
		S = AST_MakeVarDecl(T, N, V);
	}

statement(S) ::= lcName(N) SETEQUAL value(V) SEMICOLON.
	{
		S = AST_MakeAssignVar(AST_MakeLocalVar(N), V);
	}

statement(S) ::= AT lcName(N) SETEQUAL value(V) SEMICOLON.
	{
		S = AST_MakeAssignVar(AST_MakeThisVar(N), V);
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
		V = AST_MakeVarAccess(AST_MakeLocalVar(N));
	}

precision7(V) ::= AT lcName(N).
	{
		V = AST_MakeVarAccess(AST_MakeThisVar(N));
	}

precision7(V) ::= BOOLCONSTANT(C).
	{
		V = AST_MakeBoolConstant((C).value.boolValue);
	}
	
precision7(V) ::= INTCONSTANT(C).
	{
		V = AST_MakeIntConstant((C).value.intValue);
	}
	
precision7(V) ::= FLOATCONSTANT(C).
	{
		V = AST_MakeFloatConstant((C).value.floatValue);
	}

precision7(V) ::= ucName(N) LROUNDBRACKET valueList(VL) RROUNDBRACKET.
	{
		V = AST_MakeConstruct(N, NULL, VL);
	}

precision7(V) ::= ucName(TN) COLON ucName(CN) LROUNDBRACKET valueList(VL) RROUNDBRACKET.
	{
		V = AST_MakeConstruct(TN, CN, VL);
	}

precision6(V) ::= precision7(VAL).
	{
		V = VAL;
	}

precision6(V) ::= precision6(S) DOT varName(N).
	{
		V = AST_MakeMemberAccess(S, N);
	}

precision6(V) ::= precision6(O) DOT methodName(N) LROUNDBRACKET valueList(P) RROUNDBRACKET.
	{
		V = AST_MakeMethodCall(O, N, P);
	}

precision6(V) ::= precision6(SP) PTRACCESS varName(N).
	{
		V = AST_MakeMemberAccess(AST_MakeUnary(AST_UNARY_DEREF, SP), N);
	}

precision6(V) ::= precision6(OP) PTRACCESS methodName(N) LROUNDBRACKET valueList(P) RROUNDBRACKET.
	{
		V = AST_MakeMethodCall(AST_MakeUnary(AST_UNARY_DEREF, OP), N, P);
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
		V = AST_MakeUnary(AST_UNARY_NEGATE, VAL);
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


