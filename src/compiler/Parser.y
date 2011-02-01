/* Parser */

%{

#include <iostream>
#include <string>
#include "Method.h"
#include "ProxyMethodDef.h"
#include "Tree.h"
#include "List.h"
#include "Type.h"
#include "ParamSpec.h"
#include "Context.h"
#include "InternalClass.h"
#include "ExternalClass.h"
#include "Interface.h"
#include "Scope.h"
#include "If.h"
#include "VariableDeclare.h"
#include "Return.h"
#include "Variable.h"
#include "Construct.h"
#include "DirectConstruct.h"
#include "MethodCall.h"

int yyerror(const char *s);
int yylex(void);

Locic::Context * globalContext;
Locic::InternalClass * globalInternalClass;
Locic::ExternalClass * globalExternalClass;
Locic::Interface * globalInterface;

%}

%union{
	std::string * string;
	Locic::Tree * tree;
	Locic::List<Locic::Tree> * treeList;

	Locic::ProxyType * type;
	Locic::List<Locic::ProxyType> * typeList;
	Locic::ParamSpec * paramSpecList;
}

%token <tree> FLOAT
%token <tree> INT
%token <tree> BOOL
%token <tree> STRING
%token <string> TYPENAME
%token <string> VARNAME

%token PTRACCESS
%token ISEQUAL
%token NOTEQUAL
%token GREATEROREQUAL
%token LESSOREQUAL

%token IF
%token ELSE
%token WHILE
%token FOR

%token EXTERNAL

%type <string> MethodName

%type <type> Type
%type <typeList> TypeList
%type <typeList> ParamTypes
%type <paramSpecList> ParamSpec
%type <paramSpecList> ParamSpecList

%type <tree> Scope
%type <treeList> ScopeExpressions

%type <tree> MethodExpr

%type <treeList> ExpressionList
%type <treeList> MethodCallParameters
%type <tree> Precision0
%type <tree> Precision1
%type <tree> Precision2
%type <tree> Precision3
%type <tree> Precision4
%type <tree> Precision5
%type <tree> Precision6
%type <tree> Precision7
%type <tree> Expression

%%

start:
	AllowEndOfLine File AllowEndOfLine
	;

File:
	TypeDeclaration
	| File EndOfLine TypeDeclaration
	;

TypeDeclaration:
	TYPENAME '(' ')' '{'
		{
			globalInternalClass = new Locic::InternalClass($1);
		}
	AllowEndOfLine Class AllowEndOfLine '}'
		{
			globalContext->AddClass(globalInternalClass);
		}
	| EXTERNAL TYPENAME
		{
			globalExternalClass = new Locic::ExternalClass($2);
		}
	'{' AllowEndOfLine ExternalClass AllowEndOfLine '}'
		{
			globalContext->AddClass(globalExternalClass);
		}
	| TYPENAME '{'
		{
			globalInterface = new Locic::Interface($1);
		}
	AllowEndOfLine Interface AllowEndOfLine '}'
		{
			globalContext->AddInterface(globalInterface);
		}
	;

TypeList:
	Type
		{
			$$ = new Locic::List<Locic::ProxyType>($1);
		}
	| TypeList ',' Type
		{
			$$ = ($1)->Add($3);
		}
	;

ExpressionList:
	Expression
		{
			$$ = new Locic::List<Locic::Tree>($1);
		}
	| ExpressionList ',' Expression
		{
			$$ = ($1)->Add($3);
		}
	;

MethodCallParameters:
	/* empty */
		{
			$$ = new Locic::List<Locic::Tree>();
		}
	| ExpressionList
		{
			$$ = $1;
		}
	;

Type:
	TYPENAME
		{
			$$ = globalContext->Resolve($1);
		}
	;

ParamTypes:
	//empty
		{
			$$ = new Locic::List<Locic::ProxyType>();
		}
	| TypeList
		{
			$$ = $1;
		}
	;

Class:
	ClassMethod
	| Class EndOfLine ClassMethod
	;

ExternalClass:
	ExternalMethod
	| ExternalClass EndOfLine ExternalMethod
	;
	
Interface:
	InterfaceMethod
	| Interface EndOfLine InterfaceMethod
	;

MethodName:
	VARNAME
		{
			$$ = $1;
		}
	| '+'
		{
			$$ = new std::string("OpPlus");
		}
	| '-'
		{
			$$ = new std::string("OpMinus");
		}
	| '*'
		{
			$$ = new std::string("OpMultiply");
		}
	| '/'
		{
			$$ = new std::string("OpDivide");
		}
	| ISEQUAL
		{
			$$ = new std::string("OpIsEqual");
		}
	;
	
MethodExpr:
	Scope
		{
			$$ = $1;
		}
	| '=' Expression
		{
			$$ = $2;
		}
	;

ClassMethod:
	TYPENAME '(' ParamSpec ')' MethodExpr
		{
			globalInternalClass->Add(new Locic::Method($1, $3, $5));
		}
	| MethodName '(' ParamSpec ')' MethodExpr
		{
			globalInternalClass->Add(new Locic::Method($1, $3, $5));
		}
	;

ExternalMethod:
	TYPENAME '(' ParamTypes ')'
		{
			globalExternalClass->Add(new Locic::ProxyMethodDef(globalContext->Resolve(globalExternalClass->Name()), $1, $3));
		}
	| Type MethodName '(' ParamTypes ')'
		{
			globalExternalClass->Add(new Locic::ProxyMethodDef($1, $2, $4));
		}
	;

InterfaceMethod:
	Type MethodName '(' ParamTypes ')'
		{
			globalInterface->Add(new Locic::ProxyMethodDef($1, $2, $4));
		}
	;

ParamSpec:
	ParamSpecList
		{
			$$ = $1;
		}
	| /* empty */
		{
			$$ = new Locic::ParamSpec();
		}
	;
	
ParamSpecList:
	Type VARNAME
		{
			$$ = new Locic::ParamSpec($1, $2);
		}
	| ParamSpecList ',' Type VARNAME
		{
			$$ = ($1)->Add($3, $4);
		}
	;

EndOfLine:
	';'
	| EndOfLine ';'
	;
	
AllowEndOfLine:
	/* empty */
	| EndOfLine
	;

Scope:
	'{' AllowEndOfLine ScopeExpressions AllowEndOfLine '}'
		{
			$$ = new Locic::Scope($3);
		}
	;
	
ScopeExpressions:
	Expression
		{
			$$ = new Locic::List<Locic::Tree>($1);
		}
	| ScopeExpressions EndOfLine Expression
		{
			$$ = ($1)->Add($3);
		}
	;

Precision7:
	'(' Precision0 ')'
		{
			$$ = $2;
		}
	| Scope
		{
			$$ = $1;
		}
	| VARNAME
		{
			$$ = new Locic::Variable($1);
		}
/*	| '@' VARNAME
		{
			$$ = new Locic::GetMemberVar($2);
		}*/
	| INT
		{
			$$ = $1;
		}
    | BOOL
		{
			$$ = $1;
		}
	| FLOAT
		{
			$$ = $1;
		}
	| STRING
		{
			$$ = $1;
		}
	| Type ':' TYPENAME '(' MethodCallParameters ')'
		{
			globalInternalClass->Require($1);			
			$$ = new Locic::Construct($1, $3, $5);
		}
	| Type '(' MethodCallParameters ')'
		{
			$$ = new Locic::DirectConstruct($1, $3);
		}
	;

Precision6:
	Precision7
		{
			$$ = $1;
		}
/*	| Precision6 '.' VARNAME '(' MethodCallParameters ')'
		{
			$$ = new Locic::MethodCall($1, $3, $5);
		}*/
	;

Precision5:
	Precision6
		{
			$$ = $1;
		}
	| '+' Precision5
		{
			$$ = new Locic::MethodCall($2, new std::string("OpPlus"), new Locic::List<Locic::Tree>());
		}
	| '-' Precision5
		{
			$$ = new Locic::MethodCall($2, new std::string("OpMinus"), new Locic::List<Locic::Tree>());
		}
	| '!' Precision5
		{
			$$ = new Locic::MethodCall($2, new std::string("OpNot"), new Locic::List<Locic::Tree>());
		}
	;

Precision4:
	Precision5
		{
			$$ = $1;
		}
	| Precision4 '*' Precision5
		{
			$$ = new Locic::MethodCall($1, new std::string("OpMultiply"), new Locic::List<Locic::Tree>($3));
		}
	| Precision4 '/' Precision5
		{
			$$ = new Locic::MethodCall($1, new std::string("OpDivide"), new Locic::List<Locic::Tree>($3));
		}
	;

Precision3:
	Precision4
		{
			$$ = $1;
		}
	| Precision3 '+' Precision4
		{
			$$ = new Locic::MethodCall($1, new std::string("OpPlus"), new Locic::List<Locic::Tree>($3));
		}
	| Precision3 '-' Precision4
		{
			$$ = new Locic::MethodCall($1, new std::string("OpMinus"), new Locic::List<Locic::Tree>($3));
		}
	;

Precision2:
	Precision3
		{
			$$ = $1;
		}
	| Precision3 ISEQUAL Precision3
		{
			$$ = new Locic::MethodCall($1, new std::string("OpIsEqual"), new Locic::List<Locic::Tree>($3));
		}
	;

Precision1:
	Precision2
		{
			$$ = $1;
		}
	| IF '(' Expression ')' Expression ELSE Expression
		{
			$$ = new Locic::If($3, $5, $7);
		}
	;

Precision0:
	Precision1
		{
			$$ = $1;
		}
	| VARNAME '=' Expression
		{
			$$ = new Locic::VariableDeclare($1, $3);
		}
	;

Expression:
	Precision0
		{
			$$ = $1;
		}
	;
%%

int yyerror(const char * s){
	std::cout << "Parse Error" << std::endl;
	return 0;
}

