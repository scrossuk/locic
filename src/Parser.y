/* Parser */

%{

#include <Locic/AST.h>

int yyerror(const char *s);
int yylex(void);

%}

%union{
	char * name;
	AST_Type * type;
	AST_Value * value;
	AST_TypeVar * typeVar;
	AST_Scope * scope;
	AST_Statement * statement;
	AST_Type * type;
	AST_ClassDecl * classDecl;
	AST_ClassDef * classDef;
	AST_ClassMethodDecl * methodDecl;
	AST_ClassMethodDef * methodDef;
	AST_File * file;
	AST_List * list;
}

%token <value> CONSTANT
%token <name> TYPENAME
%token <name> VARNAME

%token PTRACCESS
%token ISEQUAL
%token NOTEQUAL
%token GREATEROREQUAL
%token LESSOREQUAL

%token IF
%token ELSE
%token WHILE
%token FOR
%token RETURN

%token CLASS
%token INTERFACE

%token AUTO

%type <file> File

%type <classDecl> ClassDecl
%type <classDef> ClassDef

%type <list> ClassMethodDeclList
%type <methodDecl> ClassMethodDecl
%type <list> ClassMethodDefList
%type <methodDef> ClassMethodDef

%type <type> Type

%type <name> VarName
%type <name> MethodName
%type <name> TypeName

%type <typeVar> TypeVar
%type <list> NonEmptyTypeVarList
%type <list> TypeVarList

%type <value> Value
%type <list> NonEmptyValueList
%type <list> ValueList

%type <scope> Scope
%type <list> StatementList
%type <statement> Statement

%type <value> Precision0
%type <value> Precision1
%type <value> Precision2
%type <value> Precision3
%type <value> Precision4
%type <value> Precision5
%type <value> Precision6
%type <value> Precision7

%start Start

%%

Start:
	File
		{
			printf("Parsed file successfully");
		}
	;
	
File:
	// empty
		{
			$$ = AST_MakeFile();
		}
	| File ClassDecl
		{
			$$ = AST_FileAddClassDecl($1, $2);
		}
	| File ClassDef
		{
			$$ = AST_FileAddClassDef($1, $2);
		}
	;
	
ClassDecl:
	CLASS TYPENAME '{' ClassMemberDeclList '}'
		{
			$$ = AST_MakeClassDecl($2, $4);
		}
	;
	
ClassDef:
	CLASS TYPENAME '(' TypeVarList ')' '{' ClassMemberDefList '}'
		{
			$$ = AST_MakeClassDef($2, $4, $7);
		}
	;
	
VarName:
	LCNAME
		{
			$$ = $1;
		}
	;
	
MethodName:
	LCNAME
		{
			$$ = $1;
		}
	;
	
TypeName:
	UCNAME
		{
			$$ = $1;
		}
	| INTNAME
		{
			$$ = $1;
		}
	| FLOATNAME
		{
			$$ = $1;
		}
	;

Type:
	TypeName
		{
			$$ = AST_MakeNamedType(AST_TYPE_MUTABLE, $1);
		}
	| CONST TypeName
		{
			$$ = AST_MakeNamedType(AST_TYPE_CONST, $1);
		}
	| Type '*'
		{
			$$ = AST_MakePtrType(AST_TYPE_MUTABLE, $1);
		}
	| Type '*' CONST
		{
			$$ = AST_MakePtrType(AST_TYPE_CONST, $1);
		}
	;
		
ClassMethodDeclList:
	//empty
		{
			$$ = AST_ListCreate();
		}
	| ClassMethodDeclList ClassMethodDecl
		{
			$$ = AST_ListAppend($1, $2);
		}
	;

ClassMethodDefList:
	//empty
		{
			$$ = AST_ListCreate();
		}
	| ClassMethodDefList ClassMethodDef
		{
			$$ = AST_ListAppend($1, $2);
		}
	;

ClassMethodDecl:
	UCNAME '(' ParamSpec ')' ';'
		{
			$$ = AST_MakeClassMethodDecl(0, $1, $3);
		}
	| Type MethodName '(' ParamSpec ')' ';'
		{
			$$ = AST_MakeClassMethodDecl($1, $2, $4);
		}
	;

ClassMethodDef:
	UCNAME '(' ParamSpec ')' Scope
		{
			$$ = AST_MakeClassMethodDef(AST_MakeClassMethodDecl(0, $1, $3), $5);
		}
	| Type MethodName '(' ParamSpec ')' Scope
		{
			$$ = AST_MakeClassMethodDef(AST_MakeClassMethodDecl($1, $2, $4), $6);
		}
	;
	
TypeVar:
	Type VarName
		{
			$$ = AST_MakeTypeVar($1, $2);
		}
	;

TypeVarList:
	/* empty */
		{
			$$ = AST_ListCreate();
		}
	| NonEmptyTypeVarList
		{
			$$ = $1;
		}
	;
	
NonEmptyTypeVarList:
	TypeVar
		{
			$$ = AST_ListAppend(AST_ListCreate(), $1);
		}
	| NonEmptyTypeVarList ',' TypeVar
		{
			$$ = AST_ListAppend($1, $3);
		}
	;

ValueList:
	/* empty */
		{
			$$ = AST_ListCreate();
		}
	| NonEmptyValueList
		{
			$$ = $1;
		}
	;
	
NonEmptyValueList:
	Value
		{
			$$ = AST_ListAppend(AST_ListCreate(), $1);
		}
	| NonEmptyValueList ',' Value
		{
			$$ = AST_ListAppend($1, $2);
		}
	;
	
Scope:
	'{' StatementList '}'
		{
			$$ = AST_MakeScope($2);
		}
	;
	
StatementList:
	/* empty */
		{
			$$ = AST_ListCreate();
		}
	| StatementList Statement
		{
			$$ = AST_ListAppend($1, $2);
		}
	;

Statement:
	IF '(' Value ')' Scope
		{
			$$ = AST_MakeIf($3, $5, NULL);
		}
	| IF '(' Value ')' Scope ELSE Scope
		{
			$$ = AST_MakeIf($3, $5, $7);
		}
	| AUTO VARNAME '=' Value ';'
		{
			$$ = AST_MakeAutoVarDecl($2, $4);
		}
	| Type VARNAME '=' Value ';'
		{
			$$ = AST_MakeVarDecl($1, $2, $4);
		}
	| VARNAME '=' Value ';'
		{
			$$ = AST_MakeVarAssign(AST_MakeLocalVar($1), $3);
		}
	| '@' VARNAME '=' Value ';'
		{
			$$ = AST_MakeVarAssign(AST_MakeThisVar($2), $4);
		}
	| Value ';'
		{
			$$ = AST_MakeValueStmt($1);
		}
	| RETURN Value ';'
		{
			$$ = AST_MakeReturn($2);
		}
	;

Precision7:
	'(' Precision0 ')'
		{
			$$ = $1;
		}
	| VARNAME
		{
			$$ = AST_MakeVarAccess(AST_MakeLocalVar($1));
		}
	| '@' VARNAME
		{
			$$ = AST_MakeVarAccess(AST_MakeThisVar($1));
		}
	| CONSTANT
		{
			$$ = $1;
		}
	| TYPENAME '(' ValueList ')'
		{
			$$ = AST_MakeConstruct($1, 0, $3);
		}
	| TYPENAME ':' TYPENAME '(' ValueList ')'
		{
			$$ = AST_MakeConstruct($1, $3, $5);
		}
	;

Precision6:
	Precision7
		{
			$$ = $1;
		}
	| Precision6 '.' VarName
		{
			$$ = AST_MakeMemberAccess($1, $3);
		}
	| Precision6 '.' MethodName '(' MethodCallParameters ')'
		{
			$$ = AST_MakeMethodCall($1, $3, $5);
		}
	| Precision6 PTRACCESS VarName
		{
			$$ = AST_MakeMemberAccess(AST_MakeUnary(AST_UNARY_DEREF, $1), $3);
		}
	| Precision6 PTRACCESS MethodName '(' MethodCallParameters ')'
		{
			$$ = AST_MakeMethodCall(AST_MakeUnary(AST_UNARY_DEREF, $1), $3, $5);
		}
	;

Precision5:
	Precision6
		{
			$$ = $1;
		}
	| '+' Precision5
		{
			$$ = AST_MakeUnary(AST_UNARY_PLUS, $2);
		}
	| '-' Precision5
		{
			$$ = AST_MakeUnary(AST_UNARY_MINUS, $2);
		}
	| '!' Precision5
		{
			$$ = AST_MakeUnary(AST_UNARY_NEGATE, $2);
		}
	| '&' Precision5
		{
			$$ = AST_MakeUnary(AST_UNARY_ADDRESSOF, $2);
		}
	| '*' Precision5
		{
			$$ = AST_MakeUnary(AST_UNARY_DEREF, $2);
		}
	;

Precision4:
	Precision5
		{
			$$ = $1;
		}
	| Precision4 '*' Precision5
		{
			$$ = AST_MakeBinary(AST_BINARY_MULTIPLY, $1, $3);
		}
	| Precision4 '/' Precision5
		{
			$$ = AST_MakeBinary(AST_BINARY_DIVIDE, $1, $3);
		}
	;

Precision3:
	Precision4
		{
			$$ = $1;
		}
	| Precision3 '+' Precision4
		{
			$$ = AST_MakeBinary(AST_BINARY_ADD, $1, $3);
		}
	| Precision3 '-' Precision4
		{
			$$ = AST_MakeBinary(AST_BINARY_SUBTRACT, $1, $3);
		}
	;

Precision2:
	Precision3
		{
			$$ = $1;
		}
	| Precision3 ISEQUAL Precision3
		{
			$$ = AST_MakeBinary(AST_BINARY_ISEQUAL, $1, $3);
		}
	;

Precision1:
	Precision2
		{
			$$ = $1;
		}
	| Precision2 '?' Precision1 ':' Precision1
		{
			$$ = AST_MakeTernary($1, $3, $5);
		}
	;

Precision0:
	Precision1
		{
			$$ = $1;
		}
	;

Value:
	Precision0
		{
			$$ = $1;
		}
	;
%%

int yyerror(const char * s){
	printf("Error: %s\n", s);
	return 0;
}

