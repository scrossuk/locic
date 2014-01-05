/* Parser */

%{

#include <cassert>
#include <cstdio>
#include <list>
#include <string>
#include <vector>
#include <locic/AST.hpp>
#include <locic/Name.hpp>
#include <locic/SourceLocation.hpp>
#include <locic/Parser/Context.hpp>

#include "Lexer.hpp"
#include "LocationInfo.hpp"
#include "Token.hpp"

int Locic_Parser_GeneratedParser_error(locic::Parser::LocationInfo* locationInfo, void * scanner, locic::Parser::Context * parserContext, const char *s);
int Locic_Parser_GeneratedParser_lex(locic::Parser::Token * token, locic::Parser::LocationInfo* locationInfo, void * lexer, locic::Parser::Context * parserContext);

static locic::SourceLocation convertLocationInfo(const std::string& fileName, const locic::Parser::LocationInfo* locationInfo) {
	return locic::SourceLocation(fileName,
		locic::SourceRange(
			locic::SourcePosition(locationInfo->first_line, locationInfo->first_column),
			locic::SourcePosition(locationInfo->last_line, locationInfo->last_column)
		)
	);
}

#define LOC(locationInfo) (convertLocationInfo(parserContext->fileName(), (locationInfo)))

static locic::AST::Value* UnaryOp(const std::string& name, locic::AST::Node<locic::AST::Value> operand) {
	const auto paramNode = locic::AST::Node<locic::AST::ValueList>(operand.location(), new locic::AST::ValueList());
	return locic::AST::Value::FunctionCall(locic::AST::makeNode(operand.location(), locic::AST::Value::MemberAccess(operand, name)), paramNode);
}

static locic::AST::Value* BinaryOp(const std::string& name, locic::AST::Node<locic::AST::Value> leftOperand, locic::AST::Node<locic::AST::Value> rightOperand) {
	const auto paramNode = locic::AST::makeNode(rightOperand.location(), new locic::AST::ValueList(1, rightOperand));
	return locic::AST::Value::FunctionCall(locic::AST::makeNode(leftOperand.location(), locic::AST::Value::MemberAccess(leftOperand, name)), paramNode);
}

template <typename T>
T* MAKESYM(const T& value) {
	return new T(value);
}

template <typename T>
const T& GETSYM(T* value) {
	return *value;
}

%}

// ================ Options ================
%start start

// Parser must be reentrant.
%define api.pure

// Prefix generated symbols.
%define api.prefix Locic_Parser_GeneratedParser_

// Enable location information.
%locations

// Produce verbose errors.
%error-verbose

// Use the GLR parsing algorithm.
%glr-parser

// Expecting to get a certain number of shift/reduce
// and reduce/reduce conflicts.
%expect 4
%expect-rr 3

%lex-param {void * scanner}
%lex-param {locic::Parser::Context * parserContext}
%parse-param {void * scanner}
%parse-param {locic::Parser::Context * parserContext}

%union{
	// Lexer.
	std::string* lexer_str;
	locic::Constant* lexer_constant;
	
	// Names.
	std::string* str;
	
	// Const modifier.
	bool isConst;
	
	// Constants.
	locic::AST::Node<locic::Constant>* constant;
	
	// Structures.
	locic::AST::Node<locic::AST::NamespaceData>* namespaceData;
	locic::AST::Node<locic::AST::Namespace>* nameSpace;
	locic::AST::Node<locic::AST::TypeInstance>* typeInstance;
	locic::AST::Node<locic::AST::Function>* function;
	locic::AST::Node<locic::AST::FunctionList>* functionArray;
	
	// Symbol names.
	locic::AST::Node<locic::AST::SymbolElement>* symbolElement;
	locic::AST::Node<locic::AST::Symbol>* symbol;
	
	// Type information.
	locic::AST::Node<locic::AST::Type>* type;
	locic::AST::Node<locic::AST::TypeList>* typeArray;
	locic::AST::Node<locic::AST::TypeVar>* typeVar;
	locic::AST::Node<locic::AST::TypeVarList>* typeVarArray;
	locic::AST::Node<locic::AST::TemplateTypeVar>* templateTypeVar;
	locic::AST::Node<locic::AST::TemplateTypeVarList>* templateTypeVarArray;
	
	// Program code.
	locic::AST::Node<locic::AST::Scope>* scope;
	locic::AST::Node<locic::AST::Statement>* statement;
	locic::AST::Node<locic::AST::StatementList>* statementArray;
	
	// Values.
	locic::AST::Value::CastKind castKind;
	locic::AST::Node<locic::AST::Value>* value;
	locic::AST::Node<locic::AST::ValueList>* valueArray;
}

// ================ Terminals ================
%token <lexer_str> NAME
%token <lexer_constant> CONSTANT

%token UNKNOWN
%token ERROR
%token INTERFACE
%token SEMICOLON
%token NAMESPACE
%token UNDERSCORE
%token LCURLYBRACKET
%token RCURLYBRACKET
%token LSQUAREBRACKET
%token RSQUAREBRACKET
%token AUTO
%token STATIC
%token IMPORT
%token EXPORT
%token NEW
%token DELETE
%token MOVE
%token LVAL
%token FINAL
%token REF
%token TEMPLATE
%token TYPENAME
%token VIRTUAL
%token USING
%token ENUM
%token UNION
%token CASE
%token SWITCH
%token DEFAULT
%token CONTINUE
%token BREAK
%token EXCEPTION
%token THROW
%token TRY
%token CATCH
%token SIZEOF
%token TYPEOF
%token TYPEID
%token LROUNDBRACKET
%token RROUNDBRACKET
%token PRIMITIVE
%token STRUCT
%token CLASS
%token DATATYPE
%token COLON
%token VOID
%token CONST
%token STAR
%token COMMA
%token IF
%token ELSE
%token FOR
%token WHILE
%token SETEQUAL
%token ADDEQUAL
%token SUBEQUAL
%token MULEQUAL
%token DIVEQUAL
%token PERCENTEQUAL
%token RETURN
%token AT
%token NULLVAL
%token CONST_CAST
%token STATIC_CAST
%token DYNAMIC_CAST
%token REINTERPRET_CAST
%token IS_A
%token LTRIBRACKET
%token RTRIBRACKET
%token DOUBLE_LTRIBRACKET
%token DOUBLE_RTRIBRACKET
%token DOT
%token PTRACCESS
%token PLUS
%token MINUS
%token EXCLAIMMARK
%token AMPERSAND
%token DOUBLE_AMPERSAND
%token VERTICAL_BAR
%token DOUBLE_VERTICAL_BAR
%token FORWARDSLASH
%token PERCENT
%token ISEQUAL
%token NOTEQUAL
%token GREATEROREQUAL
%token LESSOREQUAL
%token QUESTIONMARK
%token TILDA

// ================ Non-Terminals ================
%type <nameSpace> rootNamespace
%type <namespaceData> namespaceData
%type <nameSpace> nameSpace

%type <typeInstance> typeInstance
%type <typeInstance> nonTemplatedTypeInstance

%type <str> functionName

%type <function> functionDecl
%type <function> functionDef

%type <isConst> constModifier

%type <type> staticMethodReturn

%type <function> staticMethodDecl
%type <function> staticMethodDef
%type <function> methodDecl
%type <function> methodDef
%type <functionArray> methodDeclList
%type <functionArray> methodDefList

%type <type> typePrecision3
%type <type> typePrecision2
%type <type> typePrecision1
%type <type> typePrecision0
%type <type> pointerType
%type <type> type
%type <typeArray> nonEmptyTypeList
%type <typeArray> typeList
%type <typeVar> typeVar
%type <typeVarArray> nonEmptyTypeVarList
%type <typeVarArray> typeVarList
%type <typeVarArray> structVarList
%type <templateTypeVar> templateTypeVar
%type <templateTypeVarArray> templateTypeVarList

%type <symbolElement> symbolElement
%type <symbol> symbol

%type <scope> scope
%type <statementArray> statementList
%type <statement> scopedStatement
%type <statement> normalStatement

%type <constant> constant
%type <castKind> castKind
%type <value> value
%type <valueArray> nonEmptyValueList
%type <valueArray> valueList
%type <value> precision0
%type <value> precision1
%type <value> precision2
%type <value> precision3
%type <value> precision4
%type <value> precision5
%type <value> precision6
%type <value> precision7

// ================ Rules ================
%%
start:
	rootNamespace
	{
		parserContext->fileCompleted(GETSYM($1));
	}
	;

rootNamespace:
	namespaceData
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Namespace("", GETSYM($1))));
	}
	;

namespaceData:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::NamespaceData()));
	}
	| namespaceData SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData functionDecl
	{
		(GETSYM($1))->functions.push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData functionDef
	{
		(GETSYM($1))->functions.push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData typeInstance
	{
		(GETSYM($1))->typeInstances.push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData nameSpace
	{
		(GETSYM($1))->namespaces.push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData error
	{
		parserContext->error("Invalid struct, class, function or other.", LOC(&@2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

nameSpace:
	NAMESPACE NAME LCURLYBRACKET namespaceData RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Namespace(GETSYM($2), GETSYM($4))));
	}
	;

structVarList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeVarList()));
	}
	| structVarList typeVar SEMICOLON
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| structVarList SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

// TODO: make this apply to all symbol names?
functionName:
	NAME
	{
		$$ = $1;
	}
	| MOVE
	{
		$$ = MAKESYM(std::string("move"));
	}
	;
	
functionDecl:
	type functionName LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		const bool isVarArg = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET SEMICOLON
	{
		const bool isVarArg = true;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@6));
		const bool isVarArg = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@9));
		const bool isVarArg = true;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	;
	
functionDef:
	type functionName LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Def(GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($6))));
	}
	;

constModifier:
	/* empty */
	{
		$$ = false;
	}
	| CONST
	{
		$$ = true;
	}
	;

staticMethodDecl:
	STATIC type NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticMethodDecl(GETSYM($2), GETSYM($3), GETSYM($5))));
	}
	;
	
methodDecl:
	staticMethodDecl
	{
		$$ = $1;
	}
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET constModifier SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::MethodDecl($6, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	;
	
methodDeclList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::FunctionList()));
	}
	| methodDeclList methodDecl
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

staticMethodReturn:
	/* empty */
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Auto()));
	}
	| type
	{
		$$ = $1;
	}
	;
	
staticMethodDef:
	STATIC NAME SETEQUAL DEFAULT SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::DefaultStaticMethodDef(GETSYM($2))));
	}
	| STATIC staticMethodReturn NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticMethodDef(GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($7))));
	}
	;
	
methodDef:
	staticMethodDef
	{
		$$ = $1;
	}
	| TILDA scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Destructor(GETSYM($2))));
	}
	| NAME SETEQUAL DEFAULT SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::DefaultMethodDef(GETSYM($1))));
	}
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET constModifier scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::MethodDef($6, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($7))));
	}
	;
	
methodDefList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::FunctionList()));
	}
	| methodDefList methodDef
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

templateTypeVar:
	TYPENAME NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TemplateTypeVar::Typename(GETSYM($2))));
	}
	| TYPENAME NAME COLON type
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TemplateTypeVar::TypenameSpec(GETSYM($2), GETSYM($4))));
	}
	;

templateTypeVarList:
	templateTypeVar
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TemplateTypeVarList(1, GETSYM($1))));
	}
	| templateTypeVarList COMMA templateTypeVar
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

typeInstance:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET nonTemplatedTypeInstance
	{
		(GETSYM($5))->templateVariables = GETSYM($3);
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($5)).get()));
	}
	| nonTemplatedTypeInstance
	{
		$$ = $1;
	}
	;

nonTemplatedTypeInstance:
	PRIMITIVE NAME LCURLYBRACKET methodDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Primitive(GETSYM($2), GETSYM($4))));
	}
	| STRUCT NAME LCURLYBRACKET structVarList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Struct(GETSYM($2), GETSYM($4))));
	}
	| CLASS NAME LCURLYBRACKET methodDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::ClassDecl(GETSYM($2), GETSYM($4))));
	}
	| CLASS NAME LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET methodDefList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::ClassDef(GETSYM($2), GETSYM($4), GETSYM($7))));
	}
	| INTERFACE NAME LCURLYBRACKET methodDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Interface(GETSYM($2), GETSYM($4))));
	}
	| DATATYPE NAME LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Datatype(GETSYM($2), GETSYM($4))));
	}
	;

symbolElement:
	NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement(GETSYM($1), locic::AST::makeDefaultNode<locic::AST::TypeList>())));
	}
	| NAME LTRIBRACKET nonEmptyTypeList RTRIBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement(GETSYM($1), GETSYM($3))));
	}
	| TYPENAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement("typename_type", locic::AST::makeDefaultNode<locic::AST::TypeList>())));
	}
	;
	
symbol:
	symbolElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Symbol(locic::AST::Symbol::Relative() + GETSYM($1))));
	}
	| COLON COLON symbolElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Symbol(locic::AST::Symbol::Absolute() + GETSYM($3))));
	}
	| symbol COLON COLON symbolElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Symbol(*(GETSYM($1)) + GETSYM($4))));
	}
	;
	
typePrecision3:
	VOID
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Void()));
	}
	| AUTO
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Auto()));
	}
	| symbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Object(GETSYM($1))));
	}
	| LROUNDBRACKET typePrecision1 RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Bracket(GETSYM($2))));
	}
	| STAR LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET typeList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Function(GETSYM($3), GETSYM($6))));
	}
	| STAR LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET nonEmptyTypeList COMMA DOT DOT DOT RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::VarArgFunction(GETSYM($3), GETSYM($6))));
	}
	| LROUNDBRACKET error RROUNDBRACKET
	{
		parserContext->error("Invalid type.", LOC(&@2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Auto()));
	}
	;

typePrecision2:
	typePrecision3
	{
		$$ = $1;
	}
	| CONST typePrecision2
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Const(GETSYM($2))));
	}
	| LVAL LTRIBRACKET type RTRIBRACKET typePrecision2
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Lval(GETSYM($3), GETSYM($5))));
	}
	| REF LTRIBRACKET type RTRIBRACKET typePrecision2
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Ref(GETSYM($3), GETSYM($5))));
	}
	;

pointerType:
	typePrecision1 STAR
	{
		// Create 'ptr<TYPE>'.
		auto typeList = locic::AST::makeNode(LOC(&@1), new locic::AST::TypeList(1, GETSYM($1)));
		auto symbolElement = locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement("ptr", typeList));
		auto symbol = locic::AST::makeNode(LOC(&@$), new locic::AST::Symbol(locic::AST::Symbol::Absolute() + symbolElement));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Object(symbol)));
	}
	;

typePrecision1:
	typePrecision2
	{
		$$ = $1;
	}
	| pointerType
	{
		$$ = $1;
	}
	| pointerType CONST
	{
		// Create 'const ptr<TYPE>'.
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Const(GETSYM($1))));
	}
	;

typePrecision0:
	typePrecision1
	{
		$$ = $1;
	}
	| typePrecision1 AMPERSAND
	{
		// Still a built-in type until virtual typenames are implemented.
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Reference(GETSYM($1))));
	}
	;

type:
	typePrecision0
	{
		$$ = $1;
	}
	;
	
nonEmptyTypeList:
	type
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeList(1, GETSYM($1))));
	}
	| nonEmptyTypeList COMMA type
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
typeList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeList()));
	}
	| nonEmptyTypeList
	{
		$$ = $1;
	}
	;

typeVar:
	type NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeVar::NamedVar(GETSYM($1), GETSYM($2))));
	}
	| FINAL type NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeVar::FinalNamedVar(GETSYM($2), GETSYM($3))));
	}
	| type LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeVar::PatternVar(GETSYM($1), GETSYM($3))));
	}
	| UNDERSCORE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeVar::Any()));
	}
	;
	
typeVarList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeVarList()));
	}
	| nonEmptyTypeVarList typeVar
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
nonEmptyTypeVarList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeVarList()));
	}
	| nonEmptyTypeVarList typeVar COMMA
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
valueList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::ValueList()));
	}
	| nonEmptyValueList
	{
		$$ = $1;
	}
	;
	
nonEmptyValueList:
	value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::ValueList(1, GETSYM($1))));
	}
	| nonEmptyValueList COMMA value
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = $1;
	}
	;
	
scope:
	LCURLYBRACKET statementList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Scope(GETSYM($2))));
	}
	;
	
statementList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::StatementList()));
	}
	| statementList scopedStatement
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList normalStatement SEMICOLON
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList normalStatement error
	{
		parserContext->error("Statement must be terminated with semicolon.", LOC(&@3));
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList error
	{
		parserContext->error("Invalid statement.", LOC(&@2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
scopedStatement:
	scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ScopeStmt(GETSYM($1))));
	}
	| IF LROUNDBRACKET value RROUNDBRACKET scope
	{
		// One sided if statement (i.e. nothing happens in 'else' case).
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::If(GETSYM($3), GETSYM($5), locic::AST::makeNode(LOC(&@1), new locic::AST::Scope()))));
	}
	| IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::If(GETSYM($3), GETSYM($5), GETSYM($7))));
	}
	| FOR LROUNDBRACKET typeVar COLON value RROUNDBRACKET scope
	{
		/**
		 * This code converts:
		 * for (type value_var: range_value) {
		 *     //...
		 * }
		 * 
		 * ...to:
		 * 
		 * {
		 *     auto anon_var = range_value;
		 *     while (!anon_var.empty()) {
		 *         type value_var = anon_var.front();
		 *         {
		 *             //...
		 *         }
		 *         anon_var.popFront();
		 *     }
		 * }
		 *
		 * HOWEVER, this should REALLY be done in Semantic Analysis,
		 * particularly since this doesn't handle break/continue
		 * correctly and the location information generated is nonsense.
		 */
		
		const std::string anonVariableName = parserContext->getAnonymousVariableName();
		const auto emptyTypeList = locic::AST::makeNode(LOC(&@$), new locic::AST::TypeList());
		const auto anonVariableSymbolElement = locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement(anonVariableName, emptyTypeList));
		const auto anonVariableSymbol = locic::AST::makeNode(LOC(&@$), new locic::AST::Symbol(locic::AST::Symbol::Relative() + anonVariableSymbolElement));
		
		// {
		std::vector<locic::AST::Node<locic::AST::Statement>> statements;
		
		//     auto anon_var = range_value;
		locic::AST::Node<locic::AST::TypeVar> anonVarTypeVar = locic::AST::makeNode(LOC(&@$), locic::AST::TypeVar::NamedVar(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Auto()), anonVariableName));
		statements.push_back(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::VarDecl(anonVarTypeVar, GETSYM($5))));
		
		//     while (!anon_var.empty()) {
		locic::AST::Node<locic::AST::Value> condition = locic::AST::makeNode(LOC(&@$), UnaryOp("not", locic::AST::makeNode(LOC(&@$), UnaryOp("empty", locic::AST::makeNode(LOC(&@$), locic::AST::Value::SymbolRef(anonVariableSymbol))))));
		std::vector<locic::AST::Node<locic::AST::Statement>> whileLoopStatements;
		
		//         type value_var = anon_var.front();
		whileLoopStatements.push_back(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::VarDecl(GETSYM($3), locic::AST::makeNode(LOC(&@$), UnaryOp("front", locic::AST::makeNode(LOC(&@$), locic::AST::Value::SymbolRef(anonVariableSymbol)))))));
		
		//         {
		//             //...
		//         }
		whileLoopStatements.push_back(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ScopeStmt(GETSYM($7))));
		
		//         anon_var.popFront();
		const auto anonVariableSymbolRef = locic::AST::makeNode(LOC(&@$), locic::AST::Value::SymbolRef(anonVariableSymbol));
		const auto popFrontValue = locic::AST::makeNode(LOC(&@$), UnaryOp("popFront", anonVariableSymbolRef));
		whileLoopStatements.push_back(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(popFrontValue)));
		
		//     }
		const auto whileLoopStatementsNode = locic::AST::makeNode(LOC(&@$), new locic::AST::StatementList(whileLoopStatements));
		const auto whileScopeNode = locic::AST::makeNode(LOC(&@$), new locic::AST::Scope(whileLoopStatementsNode));
		statements.push_back(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::While(condition, whileScopeNode)));
		
		// }
		const auto statementsNode = locic::AST::makeNode(LOC(&@$), new locic::AST::StatementList(statements));
		const auto scopeNode = locic::AST::makeNode(LOC(&@$), new locic::AST::Scope(statementsNode));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ScopeStmt(scopeNode)));
	}
	| WHILE LROUNDBRACKET value RROUNDBRACKET scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::While(GETSYM($3), GETSYM($5))));
	}
	;
	
normalStatement:
	/*
	 * 'dprec 2' ensures that variable name definitions
	 * are preferred over assignments when there is an
	 * ambiguity.
	 *
	 * For example, 'T * p = null' is actually ambiguous,
	 * since it could mean defining a variable p as a (null)
	 * pointer to type 'T', or it could mean assigning
	 * null to the lvalue result of 'T * p', where 'T' and 'p'
	 * are both values of some kind.
	 * 
	 * Given that multiplication shouldn't return an l-value
	 * (if it really must, use parentheses around it),
	 * variable definitions always take precedence in this case.
	 */
	typeVar SETEQUAL value %dprec 2
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::VarDecl(GETSYM($1), GETSYM($3))));
	}
	
	| value SETEQUAL value %dprec 1
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), GETSYM($3))))));
	}
	
	| value ADDEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), locic::AST::makeNode(LOC(&@$), BinaryOp("add", GETSYM($1), GETSYM($3))))))));
	}
	| value SUBEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), locic::AST::makeNode(LOC(&@$), BinaryOp("subtract", GETSYM($1), GETSYM($3))))))));
	}
	| value MULEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), locic::AST::makeNode(LOC(&@$), BinaryOp("multiply", GETSYM($1), GETSYM($3))))))));
	}
	| value DIVEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), locic::AST::makeNode(LOC(&@$), BinaryOp("divide", GETSYM($1), GETSYM($3))))))));
	}
	| precision4 PERCENTEQUAL precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), locic::AST::makeNode(LOC(&@$), BinaryOp("modulo", GETSYM($1), GETSYM($3))))))));
	}
	| value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(GETSYM($1))));
	}
	| LROUNDBRACKET VOID RROUNDBRACKET value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmtVoidCast(GETSYM($4))));
	}
	| RETURN
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ReturnVoid()));
	}
	| RETURN value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Return(GETSYM($2))));
	}
	;

constant:
	CONSTANT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), $1));
	}
	;

castKind:
	STATIC_CAST
	{
		$$ = locic::AST::Value::CAST_STATIC;
	}
	| CONST_CAST
	{
		$$ = locic::AST::Value::CAST_CONST;
	}
	| DYNAMIC_CAST
	{
		$$ = locic::AST::Value::CAST_DYNAMIC;
	}
	| REINTERPRET_CAST
	{
		$$ = locic::AST::Value::CAST_REINTERPRET;
	}
	;
	
precision7:
	LROUNDBRACKET precision0 RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Bracket(GETSYM($2))));
	}
	| symbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::SymbolRef(GETSYM($1))));
	}
	| AT NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::MemberRef(GETSYM($2))));
	}
	| AT LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::InternalConstruct(GETSYM($3))));
	}
	| constant
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::ConstantValue(GETSYM($1))));
	}
	| castKind LTRIBRACKET type COMMA type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Cast($1, GETSYM($3), GETSYM($5), GETSYM($8))));
	}
	| LVAL LTRIBRACKET type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Lval(GETSYM($3), GETSYM($6))));
	}
	| REF LTRIBRACKET type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Ref(GETSYM($3), GETSYM($6))));
	}
	;
	
precision6:
	precision7
	{
		$$ = $1;
	}
	| precision6 DOT NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::MemberAccess(GETSYM($1), GETSYM($3))));
	}
	| precision6 PTRACCESS NAME
	{
		const auto derefNode = locic::AST::makeNode(LOC(&@$), UnaryOp("deref", GETSYM($1)));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::MemberAccess(derefNode, GETSYM($3))));
	}
	| precision6 LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::FunctionCall(GETSYM($1), GETSYM($3))));
	}
	| precision6 LSQUAREBRACKET value RSQUAREBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("index", GETSYM($1), GETSYM($3))));
	}
	;
	
precision5:
	precision6
	{
		$$ = $1;
	}
	| PLUS precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("plus", GETSYM($2))));
	}
	| MINUS precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("minus", GETSYM($2))));
	}
	| EXCLAIMMARK precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("not", GETSYM($2))));
	}
	| AMPERSAND precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("address", GETSYM($2))));
	}
	| STAR precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("deref", GETSYM($2))));
	}
	| MOVE precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("move", GETSYM($2))));
	}
	;
	
precision4:
	precision5
	{
		$$ = $1;
	}
	| precision4 STAR precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("multiply", GETSYM($1), GETSYM($3))));
	}
	| precision4 FORWARDSLASH precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("divide", GETSYM($1), GETSYM($3))));
	}
	| precision4 PERCENT precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("modulo", GETSYM($1), GETSYM($3))));
	}
	;
	
precision3:
	precision4
	{
		$$ = $1;
	}
	| precision3 PLUS precision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("add", GETSYM($1), GETSYM($3))));
	}
	| precision3 MINUS precision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("subtract", GETSYM($1), GETSYM($3))));
	}
	;
	
precision2:
	precision3
	{
		$$ = $1;
	}
	| precision3 ISEQUAL precision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("isZero", locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 NOTEQUAL precision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("not", 
			locic::AST::makeNode(LOC(&@$), UnaryOp("isZero", locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	| precision3 LTRIBRACKET precision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("isNegative", locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 RTRIBRACKET precision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("isPositive", locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 LESSOREQUAL precision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("not", 
			locic::AST::makeNode(LOC(&@$), UnaryOp("isPositive", locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	| precision3 GREATEROREQUAL precision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("not", 
			locic::AST::makeNode(LOC(&@$), UnaryOp("isNegative", locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	;
	
precision1:
	precision2
	{
		$$ = $1;
	}
	| precision2 QUESTIONMARK precision1 COLON precision1
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Ternary(GETSYM($1), GETSYM($3), GETSYM($5))));
	}
	;
	
precision0:
	precision1
	{
		$$ = $1;
	}
	;
	
value:
	precision0
	{
		$$ = $1;
	}
	;
	
%%

int Locic_Parser_GeneratedParser_lex(locic::Parser::Token * token, locic::Parser::LocationInfo* locationInfo, void * lexer, locic::Parser::Context * parserContext){
	(void) parserContext;
	return locic::Parser::LexGetToken(token, locationInfo, lexer);
}

int Locic_Parser_GeneratedParser_error(locic::Parser::LocationInfo* locationInfo, void * scanner, locic::Parser::Context * parserContext, const char *s) {
	(void) scanner;
	parserContext->error(s, LOC(locationInfo));
	return 0;
}

