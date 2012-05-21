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
	parserContext->parseFailed = 1;
}

%type module { AST_Module * }

%type struct { AST_Struct * }
%type structVarList { Locic_List * }

%type classDecl { AST_ClassDecl * }
%type classDef { AST_ClassDef * }

%type functionDecl { AST_FunctionDecl * }
%type functionDef { AST_FunctionDef * }

%type classMethodDeclList { Locic_List * }
%type classMethodDefList { Locic_List * }

%type basicType { AST_BasicTypeEnum }
%type typePrecision2 { AST_Type * }
%type typePrecision1 { AST_Type * }
%type typePrecision0 { AST_Type * }
%type type { AST_Type * }
%type nonEmptyTypeList { Locic_List * }
%type typeList { Locic_List * }

%type lcName { char * }
%type ucName { char * }
%type name { char * }

%type typeVar { AST_TypeVar * }
%type nonEmptyTypeVarList { Locic_List * }
%type typeVarList { Locic_List * }

%type value { AST_Value * }
%type nonEmptyValueList { Locic_List * }
%type valueList { Locic_List * }

%type scope { AST_Scope * }
%type statementList { Locic_List * }
%type scopedStatement { AST_Statement * }
%type normalStatement { AST_Statement * }

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
	
// Nasty hack to create ERROR token (UNKNOWN can never be sent by the lexer).
start ::= UNKNOWN ERROR.

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

module(NM) ::= module(OM) struct(S).
	{
		NM = AST_ModuleAddStruct(OM, S);
	}

module(NM) ::= module(OM) SEMICOLON.
	{
		NM = OM;
	}

module(NM) ::= module(OM) error.
	{
		printf("Parser Error: Invalid struct, class, function or other.\n");
		NM = OM;
	}

struct(S) ::= STRUCT name(N) LCURLYBRACKET structVarList(VL) RCURLYBRACKET.
	{
		S = AST_MakeStruct(N, VL);
	}

structVarList(VL) ::= .
	{
		VL = Locic_List_Alloc();
	}

structVarList(VL) ::= structVarList(OVL) typeVar(V) SEMICOLON.
	{
		VL = Locic_List_Append(OVL, V);
	}

structVarList(VL) ::= structVarList(OVL) SEMICOLON.
	{
		VL = OVL;
	}
	
functionDecl(D) ::= type(T) lcName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET SEMICOLON.
	{
		D = AST_MakeFunctionDecl(T, N, P);
	}
	
functionDecl(D) ::= type(T) lcName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET error.
	{
		printf("Parser Error: Function declaration must be terminated with a semicolon.\n");
		D = AST_MakeFunctionDecl(T, N, P);
	}
	
functionDef(D) ::= type(T) lcName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET scope(S).
	{
		D = new AST::FunctionDef(AST_MakeFunctionDecl(T, N, P), S);
	}
	
classDecl(D) ::= CLASS ucName(N) LCURLYBRACKET classMethodDeclList(DL) RCURLYBRACKET.
	{
		D = new AST::ClassDecl(N, DL);
	}
	
classDef(D) ::= CLASS ucName(N) LROUNDBRACKET typeVarList(VL) RROUNDBRACKET LCURLYBRACKET classMethodDefList(DL) RCURLYBRACKET.
	{
		D = new AST::ClassDef(N, VL, DL);
	}
	
lcName(N) ::= LCNAME(NAME).
	{
		N = new std::string((NAME).str);
	}
	
ucName(N) ::= UCNAME(NAME).
	{
		N = new std::string((NAME).str);
	}
	
name(N) ::= lcName(NAME).
	{
		N = NAME;
	}
	
name(N) ::= ucName(NAME).
	{
		N = NAME;
	}
	
basicType(T) ::= BOOLNAME.
	{
		T = AST::Type::BOOLEAN;
	}
	
basicType(T) ::= INTNAME.
	{
		T = AST::Type::INTEGER;
	}
	
basicType(T) ::= FLOATNAME.
	{
		T = AST::Type::FLOAT;
	}
	
typePrecision2(T) ::= VOIDNAME.
	{
		T = AST::Type::VoidType();
	}
	
typePrecision2(T) ::= basicType(BT).
	{
		T = AST::Type::BasicType(AST::Type::MUTABLE, BT);
	}
	
typePrecision2(T) ::= ucName(N).
	{
		T = AST::Type::NamedType(AST::Type::MUTABLE, *(N));
	}
	
typePrecision2(T) ::= PERCENT lcName(N).
	{
		T = AST::Type::BasicType(AST::Type::MUTABLE, *(N));
	}
	
typePrecision2(NT) ::= LROUNDBRACKET type(T) RROUNDBRACKET.
	{
		NT = T;
	}
	
typePrecision2(NT) ::= LROUNDBRACKET type(RT) RROUNDBRACKET LROUNDBRACKET typeList(PTL) RROUNDBRACKET.
	{
		NT = AST::Type::FunctionType(AST::Type::MUTABLE, RT, *(PTL));
	}
	
typePrecision2(NT) ::= LROUNDBRACKET error RROUNDBRACKET.
	{
		printf("Parser Error: Invalid type.\n");
		NT = NULL;
	}
	
typePrecision1(NT) ::= typePrecision2(T).
	{
		NT = T;
	}
	
typePrecision1(NT) ::= CONST typePrecision2(T).
	{
		NT = (T)->applyTransitiveConst();
	}
	
typePrecision0(NT) ::= typePrecision1(T).
	{
		NT = T;
	}
	
typePrecision0(NT) ::= typePrecision0(T) STAR.
	{
		NT = AST::Type::PointerType(T);
	}

type(NT) ::= typePrecision0(T).
	{
		NT = T;
	}
	
nonEmptyTypeList(TL) ::= type(T).
	{
		TL = new std::list<AST::Type *>(1, T);
	}
	
nonEmptyTypeList(TL) ::= nonEmptyTypeList(OTL) COMMA type(T).
	{
		(OTL)->push_back(T);
		TL = OTL;
	}
	
typeList(TL) ::= .
	{
		TL = new std::list<AST::Type *>();
	}
	
typeList(TL) ::= nonEmptyTypeList(NETL).
	{
		TL = NETL;
	}
	
classMethodDeclList(DL) ::= .
	{
		DL = new std::list<AST::FunctionDecl *>();
	}
	
classMethodDeclList(DL) ::= classMethodDeclList(ODL) functionDecl(D).
	{
		(ODL)->push_back(D);
		DL = ODL;
	}
	
classMethodDefList(DL) ::= .
	{
		DL = new std::list<AST::FunctionDef *>();
	}
	
classMethodDefList(DL) ::= classMethodDefList(ODL) functionDef(D).
	{
		(ODL)->push_back(D);
		DL = ODL;
	}
	
typeVar(TV) ::= type(T) lcName(N).
	{
		TV = new AST::TypeVar(T, *(N));
	}
	
typeVarList(TVL) ::= .
	{
		TVL = new std::list<AST::TypeVar *>();
	}
	
typeVarList(TVL) ::= nonEmptyTypeVarList(L).
	{
		TVL = L;
	}
	
nonEmptyTypeVarList(TVL) ::= typeVar(TV).
	{
		TVL = new std::list<AST::TypeVar *>(1, TV);
	}
	
nonEmptyTypeVarList(TVL) ::= nonEmptyTypeVarList(L) COMMA typeVar(TV).
	{
		(L)->push_back(TV);
		TVL = TV;
	}
	
valueList(VL) ::= .
	{
		VL = new std::list<AST::Value *>();
	}
	
valueList(VL) ::= nonEmptyValueList(L).
	{
		VL = L;
	}
	
nonEmptyValueList(VL) ::= value(V).
	{
		VL = new std::list<AST::Value *>(1, V);
	}
	
nonEmptyValueList(VL) ::= nonEmptyValueList(L) COMMA value(V).
	{
		(L)->push_back(V);
		VL = L;
	}
	
scope(S) ::= LCURLYBRACKET statementList(SL) RCURLYBRACKET.
	{
		S = new AST::Scope(SL);
	}
	
statementList(SL) ::= .
	{
		SL = new std::list<AST::Statement *>();
	}
	
statementList(SL) ::= statementList(L) scopedStatement(S).
	{
		(L)->push_back(S);
		SL = L;
	}
	
statementList(SL) ::= statementList(L) normalStatement(S) SEMICOLON.
	{
		(L)->push_back(S);
		SL = L;
	}
	
statementList(SL) ::= statementList(L) normalStatement(S) error.
	{
		printf("Parser Error: Statement must be terminated with semicolon.\n");
		(L)->push_back(S);
		SL = L;
	}
	
statementList(SL) ::= statementList(L) SEMICOLON.
	{
		SL = L;
	}

statementList(SL) ::= statementList(L) error.
	{
		printf("Parser Error: Invalid statement.\n");
		SL = L;
	}
	
scopedStatement(S) ::= scope(SCOPE).
	{
		S = AST::Statement::Scope(SCOPE);
	}
	
scopedStatement(S) ::= IF LROUNDBRACKET value(V) RROUNDBRACKET scope(T).
	{
		S = AST::Statement::If(V, T, new AST::Scope());
	}
	
scopedStatement(S) ::= IF LROUNDBRACKET value(V) RROUNDBRACKET scope(T) ELSE scope(F).
	{
		S = AST::Statement::If(V, T, F);
	}
	
scopedStatement(S) ::= FOR LROUNDBRACKET type lcName COLON value(V) RROUNDBRACKET scope.
	{
		// TODO
		S = AST::Statement::Value(V);
	}
	
scopedStatement(S) ::= WHILE LROUNDBRACKET value(V) RROUNDBRACKET scope(T).
	{
		S = AST::Statement::While(V, T);
	}
	
normalStatement(S) ::= AUTO lcName(N) SETEQUAL value(V).
	{
		S = AST::Statement::AutoVarDecl(*(N), V);
	}
	
normalStatement(S) ::= type(T) lcName(N) SETEQUAL value(V).
	{
		S = AST::Statement::VarDecl(T, *(N), V);
	}

normalStatement(S) ::= value(LV) SETEQUAL value(RV).
	{
		S = AST::Statement::Assign(LV, RV);
	}

normalStatement(S) ::= value(LV) ADDEQUAL value(RV).
	{
		S = AST::Statement::Assign(LV, AST::Expression::Binary(AST::Expression::ADD, LV, RV));
	}

normalStatement(S) ::= value(LV) SUBEQUAL value(RV).
	{
		S = AST::Statement::Assign(LV, AST::Expression::Binary(AST::Expression::SUBTRACT, LV, RV));
	}

normalStatement(S) ::= value(LV) MULEQUAL value(RV).
	{
		S = AST::Statement::Assign(LV, AST::Expression::Binary(AST::Expression::MULTIPLY, LV, RV));
	}

normalStatement(S) ::= value(LV) DIVEQUAL value(RV).
	{
		S = AST::Statement::Assign(LV, AST::Expression::Binary(AST::Expression::DIVIDE, LV, RV));
	}

normalStatement(S) ::= value(V).
	{
		S = AST::Statement::Value(V);
	}

normalStatement(S) ::= RETURN.
	{
		S = AST::Statement::Return(NULL);
	}

normalStatement(S) ::= RETURN value(V).
	{
		S = AST::Statement::Return(V);
	}
	
precision7(V) ::= LROUNDBRACKET precision0(BV) RROUNDBRACKET.
	{
		V = BV;
	}

precision7(V) ::= lcName(N).
	{
		V = AST::Expression::Var(AST::Var::Local(N));
	}

precision7(V) ::= AT lcName(N).
	{
		V = AST::Expression::Var(AST::Var::LocalMember(N));
	}

precision7(V) ::= BOOLCONSTANT(C).
	{
		V = AST::Expression::BoolConstant((C).boolValue);
	}
	
precision7(V) ::= INTCONSTANT(C).
	{
		V = AST::Expression::IntConstant((C).intValue);
	}
	
precision7(V) ::= FLOATCONSTANT(C).
	{
		V = AST::Expression::FloatConstant((C).floatValue);
	}
	
precision7(V) ::= NULL.
	{
		V = AST::Expression::NullConstant();
	}

precision7(V) ::= ucName(N) LROUNDBRACKET valueList(VL) RROUNDBRACKET.
	{
		V = AST::Expression::Construct(*(N), "", VL);
	}

precision7(V) ::= ucName(TN) COLON ucName(CN) LROUNDBRACKET valueList(VL) RROUNDBRACKET.
	{
		V = AST::Expression::Construct(TN, CN, VL);
	}
	
precision7(V) ::= CAST LTRIBRACKET type(T) RTRIBRACKET LROUNDBRACKET value(VAL) RROUNDBRACKET.
	{
		V = AST::Expression::Cast(T, VAL);
	}

precision6(V) ::= precision7(VAL).
	{
		V = VAL;
	}

precision6(V) ::= precision6(S) DOT lcName(N).
	{
		V = AST::Expression::MemberAccess(S, N);
	}

precision6(V) ::= precision6(SP) PTRACCESS lcName(N).
	{
		V = AST::Expression::MemberAccess(AST::Expression::Unary(AST::Expression::DEREF, SP), N);
	}

precision6(V) ::= precision6(F) LROUNDBRACKET valueList(P) RROUNDBRACKET.
	{
		V = AST::Expression::FunctionCall(F, P);
	}
	
precision5(V) ::= precision6(VAL).
	{
		V = VAL;
	}

precision5(V) ::= PLUS precision5(VAL).
	{
		V = AST::Expression::Unary(AST::Expression::PLUS, VAL);
	}

precision5(V) ::= MINUS precision5(VAL).
	{
		V = AST::Expression::Unary(AST::Expression::MINUS, VAL);
	}

precision5(V) ::= EXCLAIMMARK precision5(VAL).
	{
		V = AST::Expression::Unary(AST::Expression::NOT, VAL);
	}

precision5(V) ::= AMPERSAND precision5(VAL).
	{
		V = AST::Expression::Unary(AST::Expression::ADDRESSOF, VAL);
	}

precision5(V) ::= STAR precision5(VAL).
	{
		V = AST::Expression::Unary(AST::Expression::DEREF, VAL);
	}

precision4(V) ::= precision5(VAL).
	{
		V = VAL;
	}

precision4(V) ::= precision4(L) STAR precision5(R).
	{
		V = AST::Expression::Binary(AST::Expression::MULTIPLY, L, R);
	}

precision4(V) ::= precision4(L) FORWARDSLASH precision5(R).
	{
		V = AST::Expression::Binary(AST::Expression::DIVIDE, L, R);
	}

precision3(V) ::= precision4(VAL).
	{
		V = VAL;
	}

precision3(V) ::= precision3(L) PLUS precision4(R).
	{
		V = AST::Expression::Binary(AST::Expression::ADD, L, R);
	}

precision3(V) ::= precision3(L) MINUS precision4(R).
	{
		V = AST::Expression::Binary(AST::Expression::SUBTRACT, L, R);
	}

precision2(V) ::= precision3(VAL).
	{
		V = VAL;
	}

precision2(V) ::= precision3(L) ISEQUAL precision3(R).
	{
		V = AST::Expression::Binary(AST::Expression::ISEQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) NOTEQUAL precision3(R).
	{
		V = AST::Expression::Binary(AST::Expression::NOTEQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) LTRIBRACKET precision3(R).
	{
		V = AST::Expression::Binary(AST::Expression::LESSTHAN, L, R);
	}
	
precision2(V) ::= precision3(L) RTRIBRACKET precision3(R).
	{
		V = AST::Expression::Binary(AST::Expression::GREATERTHAN, L, R);
	}
	
precision2(V) ::= precision3(L) GREATEROREQUAL precision3(R).
	{
		V = AST::Expression::Binary(AST::Expression::GREATEROREQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) LESSOREQUAL precision3(R).
	{
		V = AST::Expression::Binary(AST::Expression::LESSOREQUAL, L, R);
	}

precision1(V) ::= precision2(VAL).
	{
		V = VAL;
	}

precision1(V) ::= precision2(C) QUESTIONMARK precision1(T) COLON precision1(F).
	{
		V = AST::Expression::Ternary(C, T, F);
	}

precision0(V) ::= precision1(VAL).
	{
		V = VAL;
	}

value(V) ::= precision0(VAL).
	{
		V = VAL;
	}


