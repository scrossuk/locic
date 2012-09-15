%include {#include <cassert>}
%include {#include <cstdio>}
%include {#include <list>}
%include {#include <string>}
%include {#include <Locic/AST.hpp>}
%include {#include <Locic/ParserContext.hpp>}
%include {#include <Locic/Token.hpp>}

%name Locic_Parse
%extra_argument { Locic::ParserContext * parserContext }
%start_symbol start
%token_prefix LOCIC_TOKEN_
%token_type { Locic::Token }

%parse_accept {
	//printf("Success!\n");
}

%parse_failure {
	//printf("Failure!\n");
	parserContext->parseFailed = true;
}

%syntax_error {
	printf("Syntax error on line %d\n", (int) parserContext->lineNumber);
	parserContext->parseFailed = true;
}

%type module { AST::Module * }

%type structVarList { std::vector<AST::TypeVar *> * }

%type typeInstance { AST::TypeInstance * }

%type constructorDecl { AST::Function * }
%type constructorDef { AST::Function * }
%type functionDecl { AST::Function * }
%type functionDef { AST::Function * }
%type methodDeclList { std::vector<AST::Function *> * }
%type methodDefList { std::vector<AST::Function *> * }
%type constructorDeclList { std::vector<AST::Function *> * }
%type constructorDefList { std::vector<AST::Function *> * }

%type basicType { AST::Type::BasicType::TypeEnum }
%type typePrecision2 { AST::Type * }
%type typePrecision1 { AST::Type * }
%type typePrecision0 { AST::Type * }
%type type { AST::Type * }
%type nonEmptyTypeList { std::vector<AST::Type *> * }
%type typeList { std::vector<AST::Type *> * }

%type lcName { std::string * }
%type ucName { std::string * }
%type name { std::string * }

%type typeVar { AST::TypeVar * }
%type nonEmptyTypeVarList { std::vector<AST::TypeVar *> * }
%type typeVarList { std::vector<AST::TypeVar *> * }

%type value { AST::Value * }
%type nonEmptyValueList { std::vector<AST::Value *> * }
%type valueList { std::vector<AST::Value *> * }

%type scope { AST::Scope * }
%type statementList { std::vector<AST::Statement *> * }
%type scopedStatement { AST::Statement * }
%type normalStatement { AST::Statement * }

%type precision0 { AST::Value * }
%type precision1 { AST::Value * }
%type precision2 { AST::Value * }
%type precision3 { AST::Value * }
%type precision4 { AST::Value * }
%type precision5 { AST::Value * }
%type precision6 { AST::Value * }
%type precision7 { AST::Value * }
	
start ::= module(M) .
	{
		printf("Completed parsing\n");
		parserContext->modules.push_back(M);
	}
	
// Nasty hack to create ERROR token (UNKNOWN can never be sent by the lexer).
start ::= UNKNOWN ERROR.

module(M) ::= .
	{
		M = new AST::Module(parserContext->currentFileName);
	}
	
module(M) ::= INTERFACE.
	{
		M = new AST::Module(parserContext->currentFileName);
	}
	
module(NM) ::= module(OM) functionDecl(F).
	{
		(OM)->functions.push_back(F);
		NM = OM;
	}

module(NM) ::= module(OM) functionDef(F).
	{
		(OM)->functions.push_back(F);
		NM = OM;
	}

module(NM) ::= module(OM) typeInstance(T).
	{
		(OM)->typeInstances.push_back(T);
		NM = OM;
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

structVarList(VL) ::= .
	{
		VL = new std::vector<AST::TypeVar *>();
	}

structVarList(VL) ::= structVarList(OVL) typeVar(V) SEMICOLON.
	{
		(OVL)->push_back(V);
		VL = OVL;
	}

structVarList(VL) ::= structVarList(OVL) SEMICOLON.
	{
		VL = OVL;
	}

constructorDecl(C) ::= AUTO ucName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET SEMICOLON.
	{
		C = AST::Function::Decl(AST::Type::UndefinedType(), *(N), *(P));
	}

constructorDef(C) ::= AUTO ucName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET scope(S).
	{
		C = AST::Function::Def(AST::Type::UndefinedType(), *(N), *(P), S);
	}
	
functionDecl(F) ::= type(T) lcName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET SEMICOLON.
	{
		F = AST::Function::Decl(T, *(N), *(P));
	}
	
functionDecl(F) ::= type(T) lcName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET error.
	{
		printf("Parser Error: Function declaration must be terminated with a semicolon.\n");
		F = AST::Function::Decl(T, *(N), *(P));
	}
	
functionDef(F) ::= type(T) lcName(N) LROUNDBRACKET typeVarList(P) RROUNDBRACKET scope(S).
	{
		F = AST::Function::Def(T, *(N), *(P), S);
	}

typeInstance(T) ::= STRUCT name(N) LCURLYBRACKET structVarList(VL) RCURLYBRACKET.
	{
		T = AST::TypeInstance::Struct(*(N), *(VL));
	}
	
typeInstance(T) ::= CLASS ucName(N) LCURLYBRACKET constructorDeclList(CL) methodDeclList(FL) RCURLYBRACKET.
	{
		T = AST::TypeInstance::ClassDecl(*(N), *(CL), *(FL));
	}
	
typeInstance(T) ::= CLASS ucName(N) LROUNDBRACKET typeVarList(VL) RROUNDBRACKET LCURLYBRACKET constructorDefList(CL) methodDefList(FL) RCURLYBRACKET.
	{
		T = AST::TypeInstance::ClassDef(*(N), *(VL), *(CL), *(FL));
	}
	
lcName(N) ::= LCNAME(NAME).
	{
		N = (NAME).str;
	}
	
ucName(N) ::= UCNAME(NAME).
	{
		N = (NAME).str;
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
		T = AST::Type::BasicType::BOOLEAN;
	}
	
basicType(T) ::= INTNAME.
	{
		T = AST::Type::BasicType::INTEGER;
	}
	
basicType(T) ::= FLOATNAME.
	{
		T = AST::Type::BasicType::FLOAT;
	}
	
typePrecision2(T) ::= VOIDNAME.
	{
		T = AST::Type::VoidType();
	}
	
typePrecision2(T) ::= basicType(BT).
	{
		const bool isMutable = true;
		T = AST::Type::Basic(isMutable, BT);
	}
	
typePrecision2(T) ::= ucName(N).
	{
		const bool isMutable = true;
		T = AST::Type::Named(isMutable, *(N));
	}
	
typePrecision2(T) ::= PERCENT lcName(N).
	{
		T = AST::Type::Named(AST::Type::MUTABLE, *(N));
	}
	
typePrecision2(NT) ::= CONST LROUNDBRACKET type(T) RROUNDBRACKET.
	{
		NT = (T)->applyTransitiveConst();
	}
	
typePrecision2(NT) ::= LROUNDBRACKET type(RT) RROUNDBRACKET LROUNDBRACKET typeList(PTL) RROUNDBRACKET.
	{
		const bool isMutable = true;
		NT = AST::Type::Function(isMutable, RT, *(PTL));
	}
	
typePrecision2(NT) ::= CONST LROUNDBRACKET error RROUNDBRACKET.
	{
		printf("Parser Error: Invalid type.\n");
		NT = NULL;
	}

typePrecision1(NT) ::= typePrecision2(T).
	{
		NT = T;
	}
	
typePrecision1(NT) ::= typePrecision1(T) STAR.
	{
		NT = AST::Type::Pointer(T);
	}
	
typePrecision0(NT) ::= typePrecision1(T).
	{
		NT = T;
	}
	
typePrecision0(NT) ::= CONST typePrecision1(T).
	{
		NT = (T)->applyTransitiveConst();
	}

type(NT) ::= typePrecision0(T).
	{
		NT = T;
	}
	
nonEmptyTypeList(TL) ::= type(T).
	{
		TL = new std::vector<AST::Type *>(1, T);
	}
	
nonEmptyTypeList(TL) ::= nonEmptyTypeList(OTL) COMMA type(T).
	{
		(OTL)->push_back(T);
		TL = OTL;
	}
	
typeList(TL) ::= .
	{
		TL = new std::vector<AST::Type *>();
	}
	
typeList(TL) ::= nonEmptyTypeList(NETL).
	{
		TL = NETL;
	}
	
methodDeclList(DL) ::= .
	{
		DL = new std::vector<AST::Function *>();
	}
	
methodDeclList(L) ::= methodDeclList(OL) functionDecl(F).
	{
		(OL)->push_back(F);
		L = OL;
	}
	
methodDefList(DL) ::= .
	{
		DL = new std::vector<AST::Function *>();
	}
	
methodDefList(L) ::= methodDefList(OL) functionDef(F).
	{
		(OL)->push_back(F);
		L = OL;
	}

constructorDeclList(DL) ::= .
	{
		DL = new std::vector<AST::Function *>();
	}

constructorDeclList(L) ::= constructorDeclList(OL) constructorDecl(C).
	{
		(OL)->push_back(C);
		L = OL;
	}
	
constructorDefList(DL) ::= .
	{
		DL = new std::vector<AST::Function *>();
	}

constructorDefList(L) ::= constructorDefList(OL) constructorDef(C).
	{
		(OL)->push_back(C);
		L = OL;
	}
	
typeVar(TV) ::= type(T) lcName(N).
	{
		TV = new AST::TypeVar(T, *(N));
	}
	
typeVarList(TVL) ::= .
	{
		TVL = new std::vector<AST::TypeVar *>();
	}
	
typeVarList(TVL) ::= nonEmptyTypeVarList(L).
	{
		TVL = L;
	}
	
nonEmptyTypeVarList(TVL) ::= typeVar(TV).
	{
		TVL = new std::vector<AST::TypeVar *>(1, TV);
	}
	
nonEmptyTypeVarList(TVL) ::= nonEmptyTypeVarList(L) COMMA typeVar(TV).
	{
		(L)->push_back(TV);
		TVL = L;
	}
	
valueList(VL) ::= .
	{
		VL = new std::vector<AST::Value *>();
	}
	
valueList(VL) ::= nonEmptyValueList(L).
	{
		VL = L;
	}
	
nonEmptyValueList(VL) ::= value(V).
	{
		VL = new std::vector<AST::Value *>(1, V);
	}
	
nonEmptyValueList(VL) ::= nonEmptyValueList(L) COMMA value(V).
	{
		(L)->push_back(V);
		VL = L;
	}
	
scope(S) ::= LCURLYBRACKET statementList(SL) RCURLYBRACKET.
	{
		S = new AST::Scope(*(SL));
	}
	
statementList(SL) ::= .
	{
		SL = new std::vector<AST::Statement *>();
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
		S = AST::Statement::ScopeStmt(SCOPE);
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
		S = AST::Statement::ValueStmt(V);
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
		S = AST::Statement::Assign(LV, AST::Value::BinaryOp(AST::Value::Binary::ADD, LV, RV));
	}

normalStatement(S) ::= value(LV) SUBEQUAL value(RV).
	{
		S = AST::Statement::Assign(LV, AST::Value::BinaryOp(AST::Value::Binary::SUBTRACT, LV, RV));
	}

normalStatement(S) ::= value(LV) MULEQUAL value(RV).
	{
		S = AST::Statement::Assign(LV, AST::Value::BinaryOp(AST::Value::Binary::MULTIPLY, LV, RV));
	}

normalStatement(S) ::= value(LV) DIVEQUAL value(RV).
	{
		S = AST::Statement::Assign(LV, AST::Value::BinaryOp(AST::Value::Binary::DIVIDE, LV, RV));
	}

normalStatement(S) ::= value(V).
	{
		S = AST::Statement::ValueStmt(V);
	}

normalStatement(S) ::= RETURN.
	{
		S = AST::Statement::ReturnVoid();
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
		V = AST::Value::VarValue(AST::Var::Local(*(N)));
	}

precision7(V) ::= AT lcName(N).
	{
		V = AST::Value::VarValue(AST::Var::Member(*(N)));
	}

precision7(V) ::= BOOLCONSTANT(C).
	{
		V = AST::Value::BoolConstant((C).boolValue);
	}
	
precision7(V) ::= INTCONSTANT(C).
	{
		V = AST::Value::IntConstant((C).intValue);
	}
	
precision7(V) ::= FLOATCONSTANT(C).
	{
		V = AST::Value::FloatConstant((C).floatValue);
	}
	
precision7(V) ::= NULL.
	{
		V = AST::Value::NullConstant();
	}

precision7(V) ::= ucName(N) LROUNDBRACKET valueList(VL) RROUNDBRACKET.
	{
		V = AST::Value::FunctionCall(AST::Value::Construct(*(N), "Default"), *(VL));
	}

precision7(V) ::= ucName(TN) COLON ucName(CN) LROUNDBRACKET valueList(VL) RROUNDBRACKET.
	{
		V = AST::Value::FunctionCall(AST::Value::Construct(*(TN), *(CN)), *(VL));
	}
	
precision7(V) ::= CAST LTRIBRACKET type(T) RTRIBRACKET LROUNDBRACKET value(VAL) RROUNDBRACKET.
	{
		V = AST::Value::Cast(T, VAL);
	}

precision6(V) ::= precision7(VAL).
	{
		V = VAL;
	}

precision6(V) ::= precision6(S) DOT lcName(N).
	{
		V = AST::Value::MemberAccess(S, *(N));
	}

precision6(V) ::= precision6(SP) PTRACCESS lcName(N).
	{
		V = AST::Value::MemberAccess(AST::Value::UnaryOp(AST::Value::Unary::DEREF, SP), *(N));
	}

precision6(V) ::= precision6(F) LROUNDBRACKET valueList(P) RROUNDBRACKET.
	{
		V = AST::Value::FunctionCall(F, *(P));
	}
	
precision5(V) ::= precision6(VAL).
	{
		V = VAL;
	}

precision5(V) ::= PLUS precision5(VAL).
	{
		V = AST::Value::UnaryOp(AST::Value::Unary::PLUS, VAL);
	}

precision5(V) ::= MINUS precision5(VAL).
	{
		V = AST::Value::UnaryOp(AST::Value::Unary::MINUS, VAL);
	}

precision5(V) ::= EXCLAIMMARK precision5(VAL).
	{
		V = AST::Value::UnaryOp(AST::Value::Unary::NOT, VAL);
	}

precision5(V) ::= AMPERSAND precision5(VAL).
	{
		V = AST::Value::UnaryOp(AST::Value::Unary::ADDRESSOF, VAL);
	}

precision5(V) ::= STAR precision5(VAL).
	{
		V = AST::Value::UnaryOp(AST::Value::Unary::DEREF, VAL);
	}

precision4(V) ::= precision5(VAL).
	{
		V = VAL;
	}

precision4(V) ::= precision4(L) STAR precision5(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::MULTIPLY, L, R);
	}

precision4(V) ::= precision4(L) FORWARDSLASH precision5(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::DIVIDE, L, R);
	}

precision3(V) ::= precision4(VAL).
	{
		V = VAL;
	}

precision3(V) ::= precision3(L) PLUS precision4(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::ADD, L, R);
	}

precision3(V) ::= precision3(L) MINUS precision4(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::SUBTRACT, L, R);
	}

precision2(V) ::= precision3(VAL).
	{
		V = VAL;
	}

precision2(V) ::= precision3(L) ISEQUAL precision3(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::ISEQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) NOTEQUAL precision3(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::NOTEQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) LTRIBRACKET precision3(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::LESSTHAN, L, R);
	}
	
precision2(V) ::= precision3(L) RTRIBRACKET precision3(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::GREATERTHAN, L, R);
	}
	
precision2(V) ::= precision3(L) GREATEROREQUAL precision3(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::GREATEROREQUAL, L, R);
	}
	
precision2(V) ::= precision3(L) LESSOREQUAL precision3(R).
	{
		V = AST::Value::BinaryOp(AST::Value::Binary::LESSOREQUAL, L, R);
	}

precision1(V) ::= precision2(VAL).
	{
		V = VAL;
	}

precision1(V) ::= precision2(C) QUESTIONMARK precision1(T) COLON precision1(F).
	{
		V = AST::Value::Ternary(C, T, F);
	}

precision0(V) ::= precision1(VAL).
	{
		V = VAL;
	}

value(V) ::= precision0(VAL).
	{
		V = VAL;
	}


