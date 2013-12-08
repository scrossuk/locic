/* Parser */

%{

#include <cassert>
#include <cstdio>
#include <list>
#include <string>
#include <vector>
#include <Locic/AST.hpp>
#include <Locic/Name.hpp>
#include <Locic/SourceLocation.hpp>
#include <Locic/Parser/Context.hpp>
#include <Locic/Parser/Lexer.hpp>
#include <Locic/Parser/LocationInfo.hpp>
#include <Locic/Parser/Token.hpp>

int Locic_Parser_GeneratedParser_error(Locic::Parser::LocationInfo* locationInfo, void * scanner, Locic::Parser::Context * parserContext, const char *s);
int Locic_Parser_GeneratedParser_lex(Locic::Parser::Token * token, Locic::Parser::LocationInfo* locationInfo, void * lexer, Locic::Parser::Context * parserContext);

static Locic::SourceLocation convertLocationInfo(const std::string& fileName, const Locic::Parser::LocationInfo* locationInfo) {
	return Locic::SourceLocation(fileName,
		Locic::SourceRange(
			Locic::SourcePosition(locationInfo->first_line, locationInfo->first_column),
			Locic::SourcePosition(locationInfo->last_line, locationInfo->last_column)
		)
	);
}

#define LOC(locationInfo) (convertLocationInfo(parserContext->fileName(), (locationInfo)))

static Locic::AST::Value* UnaryOp(const std::string& name, Locic::AST::Node<Locic::AST::Value> operand) {
	const auto paramNode = Locic::AST::Node<Locic::AST::ValueList>(operand.location(), new Locic::AST::ValueList());
	return Locic::AST::Value::FunctionCall(Locic::AST::makeNode(operand.location(), Locic::AST::Value::MemberAccess(operand, name)), paramNode);
}

static Locic::AST::Value* BinaryOp(const std::string& name, Locic::AST::Node<Locic::AST::Value> leftOperand, Locic::AST::Node<Locic::AST::Value> rightOperand) {
	const auto paramNode = Locic::AST::makeNode(rightOperand.location(), new Locic::AST::ValueList(1, rightOperand));
	return Locic::AST::Value::FunctionCall(Locic::AST::makeNode(leftOperand.location(), Locic::AST::Value::MemberAccess(leftOperand, name)), paramNode);
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
%expect 2
%expect-rr 3

%lex-param {void * scanner}
%lex-param {Locic::Parser::Context * parserContext}
%parse-param {void * scanner}
%parse-param {Locic::Parser::Context * parserContext}

%union{
	// Lexer.
	std::string* lexer_str;
	Locic::Constant* lexer_constant;
	
	// Names.
	std::string* str;
	
	// Constants.
	Locic::AST::Node<Locic::Constant>* constant;
	
	// Structures.
	Locic::AST::Node<Locic::AST::NamespaceData>* namespaceData;
	Locic::AST::Node<Locic::AST::Namespace>* nameSpace;
	Locic::AST::Node<Locic::AST::TypeInstance>* typeInstance;
	Locic::AST::Node<Locic::AST::Function>* function;
	Locic::AST::Node<Locic::AST::FunctionList>* functionArray;
	
	// Symbol names.
	Locic::AST::Node<Locic::AST::SymbolElement>* symbolElement;
	Locic::AST::Node<Locic::AST::Symbol>* symbol;
	
	// Type information.
	Locic::AST::Node<Locic::AST::Type>* type;
	Locic::AST::Node<Locic::AST::TypeList>* typeArray;
	Locic::AST::Node<Locic::AST::TypeVar>* typeVar;
	Locic::AST::Node<Locic::AST::TypeVarList>* typeVarArray;
	Locic::AST::Node<Locic::AST::TemplateTypeVar>* templateTypeVar;
	Locic::AST::Node<Locic::AST::TemplateTypeVarList>* templateTypeVarArray;
	
	// Program code.
	Locic::AST::Node<Locic::AST::Scope>* scope;
	Locic::AST::Node<Locic::AST::Statement>* statement;
	Locic::AST::Node<Locic::AST::StatementList>* statementArray;
	
	// Values.
	Locic::AST::Value::CastKind castKind;
	Locic::AST::Node<Locic::AST::Value>* value;
	Locic::AST::Node<Locic::AST::ValueList>* valueArray;
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
%token VOIDNAME
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

%type <function> staticFunctionDecl
%type <function> staticFunctionDef
%type <function> classFunctionDecl
%type <function> classFunctionDef
%type <functionArray> classFunctionDeclList
%type <functionArray> classFunctionDefList

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
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::Namespace("", GETSYM($1))));
	}
	;

namespaceData:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::NamespaceData()));
	}
	| namespaceData SEMICOLON
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData functionDecl
	{
		(GETSYM($1))->functions.push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData functionDef
	{
		(GETSYM($1))->functions.push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData typeInstance
	{
		(GETSYM($1))->typeInstances.push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData nameSpace
	{
		(GETSYM($1))->namespaces.push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData error
	{
		parserContext->error("Invalid struct, class, function or other.", LOC(&@2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

nameSpace:
	NAMESPACE NAME LCURLYBRACKET namespaceData RCURLYBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::Namespace(GETSYM($2), GETSYM($4))));
	}
	;

structVarList:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::TypeVarList()));
	}
	| structVarList typeVar SEMICOLON
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| structVarList SEMICOLON
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

staticFunctionDecl:
	STATIC NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		const auto implicitAutoType = Locic::AST::makeNode(LOC(&@1), Locic::AST::Type::Undefined());
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::Decl(implicitAutoType, GETSYM($2), GETSYM($4))));
	}
	| STATIC type NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::Decl(GETSYM($2), GETSYM($3), GETSYM($5))));
	}
	;

staticFunctionDef:
	STATIC NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		const auto implicitAutoType = Locic::AST::makeNode(LOC(&@1), Locic::AST::Type::Undefined());
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::Def(implicitAutoType, GETSYM($2), GETSYM($4), GETSYM($6))));
	}
	| STATIC type NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::Def(GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($7))));
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
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::Decl(GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET SEMICOLON
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::VarArgDecl(GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@6));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::Decl(GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@9));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::VarArgDecl(GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	;
	
functionDef:
	type functionName LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::Def(GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($6))));
	}
	;
	
classFunctionDecl:
	staticFunctionDecl
	{
		$$ = $1;
	}
	| functionDecl
	{
		(GETSYM($1))->isMethod = true;
		$$ = $1;
	}
	;
	
classFunctionDef:
	staticFunctionDef
	{
		$$ = $1;
	}
	| TILDA scope
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Function::Destructor(GETSYM($2))));
	}
	| functionDef
	{
		(GETSYM($1))->isMethod = true;
		$$ = $1;
	}
	;

classFunctionDeclList:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::FunctionList()));
	}
	| classFunctionDeclList classFunctionDecl
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
classFunctionDefList:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::FunctionList()));
	}
	| classFunctionDefList classFunctionDef
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

templateTypeVar:
	TYPENAME NAME
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TemplateTypeVar::Typename(GETSYM($2))));
	}
	| TYPENAME NAME COLON type
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TemplateTypeVar::TypenameSpec(GETSYM($2), GETSYM($4))));
	}
	;

templateTypeVarList:
	templateTypeVar
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::TemplateTypeVarList(1, GETSYM($1))));
	}
	| templateTypeVarList COMMA templateTypeVar
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

typeInstance:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET nonTemplatedTypeInstance
	{
		(GETSYM($5))->templateVariables = GETSYM($3);
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($5)).get()));
	}
	| nonTemplatedTypeInstance
	{
		$$ = $1;
	}
	;

nonTemplatedTypeInstance:
	STRUCT NAME LCURLYBRACKET structVarList RCURLYBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeInstance::Struct(GETSYM($2), GETSYM($4))));
	}
	| CLASS NAME LCURLYBRACKET classFunctionDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeInstance::ClassDecl(GETSYM($2), GETSYM($4))));
	}
	| CLASS NAME LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classFunctionDefList RCURLYBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeInstance::ClassDef(GETSYM($2), GETSYM($4), GETSYM($7))));
	}
	| PRIMITIVE NAME LCURLYBRACKET classFunctionDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeInstance::Primitive(GETSYM($2), GETSYM($4))));
	}
	| INTERFACE NAME LCURLYBRACKET classFunctionDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeInstance::Interface(GETSYM($2), GETSYM($4))));
	}
	| DATATYPE NAME LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeInstance::Datatype(GETSYM($2), GETSYM($4))));
	}
	;

symbolElement:
	NAME
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::SymbolElement(GETSYM($1), Locic::AST::makeDefaultNode<Locic::AST::TypeList>())));
	}
	| NAME LTRIBRACKET nonEmptyTypeList RTRIBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::SymbolElement(GETSYM($1), GETSYM($3))));
	}
	| TYPENAME
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::SymbolElement("typename_type", Locic::AST::makeDefaultNode<Locic::AST::TypeList>())));
	}
	;
	
symbol:
	symbolElement
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::Symbol(Locic::AST::Symbol::Relative() + GETSYM($1))));
	}
	| COLON COLON symbolElement
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::Symbol(Locic::AST::Symbol::Absolute() + GETSYM($3))));
	}
	| symbol COLON COLON symbolElement
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::Symbol(*(GETSYM($1)) + GETSYM($4))));
	}
	;
	
typePrecision3:
	VOIDNAME
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Void()));
	}
	| symbol
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Object(GETSYM($1))));
	}
	| AUTO
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Undefined()));
	}
	| LROUNDBRACKET typePrecision1 RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Bracket(GETSYM($2))));
	}
	| STAR LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET typeList RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Function(GETSYM($3), GETSYM($6))));
	}
	| STAR LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET nonEmptyTypeList COMMA DOT DOT DOT RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::VarArgFunction(GETSYM($3), GETSYM($6))));
	}
	| LROUNDBRACKET error RROUNDBRACKET
	{
		parserContext->error("Invalid type.", LOC(&@2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Undefined()));
	}
	;

typePrecision2:
	typePrecision3
	{
		$$ = $1;
	}
	| CONST typePrecision3
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Const(GETSYM($2))));
	}
	;

pointerType:
	typePrecision1 STAR
	{
		// Create 'ptr<TYPE>'.
		auto typeList = Locic::AST::makeNode(LOC(&@1), new Locic::AST::TypeList(1, GETSYM($1)));
		auto symbolElement = Locic::AST::makeNode(LOC(&@$), new Locic::AST::SymbolElement("ptr", typeList));
		auto symbol = Locic::AST::makeNode(LOC(&@$), new Locic::AST::Symbol(Locic::AST::Symbol::Absolute() + symbolElement));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Object(symbol)));
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
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Const(GETSYM($1))));
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
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Reference(GETSYM($1))));
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
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::TypeList(1, GETSYM($1))));
	}
	| nonEmptyTypeList COMMA type
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
typeList:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::TypeList()));
	}
	| nonEmptyTypeList
	{
		$$ = $1;
	}
	;

typeVar:
	type NAME
	{
		const bool usesCustomLval = false;
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeVar::NamedVar(GETSYM($1), GETSYM($2), usesCustomLval)));
	}
	| LVAL type NAME
	{
		const bool usesCustomLval = true;
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeVar::NamedVar(GETSYM($2), GETSYM($3), usesCustomLval)));
	}
	| type LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeVar::PatternVar(GETSYM($1), GETSYM($3))));
	}
	| UNDERSCORE
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeVar::Any()));
	}
	;
	
typeVarList:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::TypeVarList()));
	}
	| nonEmptyTypeVarList typeVar
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
nonEmptyTypeVarList:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::TypeVarList()));
	}
	| nonEmptyTypeVarList typeVar COMMA
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
valueList:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::ValueList()));
	}
	| nonEmptyValueList
	{
		$$ = $1;
	}
	;
	
nonEmptyValueList:
	value
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::ValueList(1, GETSYM($1))));
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
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::Scope(GETSYM($2))));
	}
	;
	
statementList:
	// empty
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), new Locic::AST::StatementList()));
	}
	| statementList scopedStatement
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList normalStatement SEMICOLON
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList normalStatement error
	{
		parserContext->error("Statement must be terminated with semicolon.", LOC(&@3));
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList SEMICOLON
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList error
	{
		parserContext->error("Invalid statement.", LOC(&@2));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
scopedStatement:
	scope
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ScopeStmt(GETSYM($1))));
	}
	| IF LROUNDBRACKET value RROUNDBRACKET scope
	{
		// One sided if statement (i.e. nothing happens in 'else' case).
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::If(GETSYM($3), GETSYM($5), Locic::AST::makeNode(LOC(&@1), new Locic::AST::Scope()))));
	}
	| IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::If(GETSYM($3), GETSYM($5), GETSYM($7))));
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
		const auto emptyTypeList = Locic::AST::makeNode(LOC(&@$), new Locic::AST::TypeList());
		const auto anonVariableSymbolElement = Locic::AST::makeNode(LOC(&@$), new Locic::AST::SymbolElement(anonVariableName, emptyTypeList));
		const auto anonVariableSymbol = Locic::AST::makeNode(LOC(&@$), new Locic::AST::Symbol(Locic::AST::Symbol::Relative() + anonVariableSymbolElement));
		
		// {
		std::vector< Locic::AST::Node<Locic::AST::Statement> > statements;
		
		//     auto anon_var = range_value;
		const bool usesCustomLval = false;
		Locic::AST::Node<Locic::AST::TypeVar> anonVarTypeVar = Locic::AST::makeNode(LOC(&@$), Locic::AST::TypeVar::NamedVar(Locic::AST::makeNode(LOC(&@$), Locic::AST::Type::Undefined()), anonVariableName, usesCustomLval));
		statements.push_back(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::VarDecl(anonVarTypeVar, GETSYM($5))));
		
		//     while (!anon_var.empty()) {
		Locic::AST::Node<Locic::AST::Value> condition = Locic::AST::makeNode(LOC(&@$), UnaryOp("not", Locic::AST::makeNode(LOC(&@$), UnaryOp("empty", Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::SymbolRef(anonVariableSymbol))))));
		std::vector< Locic::AST::Node<Locic::AST::Statement> > whileLoopStatements;
		
		//         type value_var = anon_var.front();
		whileLoopStatements.push_back(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::VarDecl(GETSYM($3), Locic::AST::makeNode(LOC(&@$), UnaryOp("front", Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::SymbolRef(anonVariableSymbol)))))));
		
		//         {
		//             //...
		//         }
		whileLoopStatements.push_back(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ScopeStmt(GETSYM($7))));
		
		//         anon_var.popFront();
		const auto anonVariableSymbolRef = Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::SymbolRef(anonVariableSymbol));
		const auto popFrontValue = Locic::AST::makeNode(LOC(&@$), UnaryOp("popFront", anonVariableSymbolRef));
		whileLoopStatements.push_back(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ValueStmt(popFrontValue)));
		
		//     }
		const auto whileLoopStatementsNode = Locic::AST::makeNode(LOC(&@$), new Locic::AST::StatementList(whileLoopStatements));
		const auto whileScopeNode = Locic::AST::makeNode(LOC(&@$), new Locic::AST::Scope(whileLoopStatementsNode));
		statements.push_back(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::While(condition, whileScopeNode)));
		
		// }
		const auto statementsNode = Locic::AST::makeNode(LOC(&@$), new Locic::AST::StatementList(statements));
		const auto scopeNode = Locic::AST::makeNode(LOC(&@$), new Locic::AST::Scope(statementsNode));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ScopeStmt(scopeNode)));
	}
	| WHILE LROUNDBRACKET value RROUNDBRACKET scope
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::While(GETSYM($3), GETSYM($5))));
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
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::VarDecl(GETSYM($1), GETSYM($3))));
	}
	
	| value SETEQUAL value %dprec 1
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ValueStmt(Locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), GETSYM($3))))));
	}
	
	| value ADDEQUAL value
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ValueStmt(Locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), Locic::AST::makeNode(LOC(&@$), BinaryOp("add", GETSYM($1), GETSYM($3))))))));
	}
	| value SUBEQUAL value
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ValueStmt(Locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), Locic::AST::makeNode(LOC(&@$), BinaryOp("subtract", GETSYM($1), GETSYM($3))))))));
	}
	| value MULEQUAL value
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ValueStmt(Locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), Locic::AST::makeNode(LOC(&@$), BinaryOp("multiply", GETSYM($1), GETSYM($3))))))));
	}
	| value DIVEQUAL value
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ValueStmt(Locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), Locic::AST::makeNode(LOC(&@$), BinaryOp("divide", GETSYM($1), GETSYM($3))))))));
	}
	| precision4 PERCENTEQUAL precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ValueStmt(Locic::AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), Locic::AST::makeNode(LOC(&@$), BinaryOp("modulo", GETSYM($1), GETSYM($3))))))));
	}
	| value
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ValueStmt(GETSYM($1))));
	}
	| RETURN
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::ReturnVoid()));
	}
	| RETURN value
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Statement::Return(GETSYM($2))));
	}
	;

constant:
	CONSTANT
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), $1));
	}
	;

castKind:
	STATIC_CAST
	{
		$$ = Locic::AST::Value::CAST_STATIC;
	}
	| CONST_CAST
	{
		$$ = Locic::AST::Value::CAST_CONST;
	}
	| DYNAMIC_CAST
	{
		$$ = Locic::AST::Value::CAST_DYNAMIC;
	}
	| REINTERPRET_CAST
	{
		$$ = Locic::AST::Value::CAST_REINTERPRET;
	}
	;
	
precision7:
	LROUNDBRACKET precision0 RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::Bracket(GETSYM($2))));
	}
	| symbol
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::SymbolRef(GETSYM($1))));
	}
	| AT NAME
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::MemberRef(GETSYM($2))));
	}
	| AT LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::InternalConstruct(GETSYM($3))));
	}
	| constant
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::ConstantValue(GETSYM($1))));
	}
	| castKind LTRIBRACKET type COMMA type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::Cast($1, GETSYM($3), GETSYM($5), GETSYM($8))));
	}
	;
	
precision6:
	precision7
	{
		$$ = $1;
	}
	| precision6 DOT NAME
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::MemberAccess(GETSYM($1), GETSYM($3))));
	}
	| precision6 PTRACCESS NAME
	{
		const auto derefNode = Locic::AST::makeNode(LOC(&@$), UnaryOp("deref", GETSYM($1)));
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::MemberAccess(derefNode, GETSYM($3))));
	}
	| precision6 LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::FunctionCall(GETSYM($1), GETSYM($3))));
	}
	| precision6 LSQUAREBRACKET value RSQUAREBRACKET
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), BinaryOp("index", GETSYM($1), GETSYM($3))));
	}
	;
	
precision5:
	precision6
	{
		$$ = $1;
	}
	| PLUS precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("plus", GETSYM($2))));
	}
	| MINUS precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("minus", GETSYM($2))));
	}
	| EXCLAIMMARK precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("not", GETSYM($2))));
	}
	| AMPERSAND precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("address", GETSYM($2))));
	}
	| STAR precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("deref", GETSYM($2))));
	}
	| MOVE precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("move", GETSYM($2))));
	}
	;
	
precision4:
	precision5
	{
		$$ = $1;
	}
	| precision4 STAR precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), BinaryOp("multiply", GETSYM($1), GETSYM($3))));
	}
	| precision4 FORWARDSLASH precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), BinaryOp("divide", GETSYM($1), GETSYM($3))));
	}
	| precision4 PERCENT precision5
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), BinaryOp("modulo", GETSYM($1), GETSYM($3))));
	}
	;
	
precision3:
	precision4
	{
		$$ = $1;
	}
	| precision3 PLUS precision4
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), BinaryOp("add", GETSYM($1), GETSYM($3))));
	}
	| precision3 MINUS precision4
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), BinaryOp("subtract", GETSYM($1), GETSYM($3))));
	}
	;
	
precision2:
	precision3
	{
		$$ = $1;
	}
	| precision3 ISEQUAL precision3
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("isZero", Locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 NOTEQUAL precision3
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("not", 
			Locic::AST::makeNode(LOC(&@$), UnaryOp("isZero", Locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	| precision3 LTRIBRACKET precision3
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("isNegative", Locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 RTRIBRACKET precision3
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("isPositive", Locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 LESSOREQUAL precision3
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("not", 
			Locic::AST::makeNode(LOC(&@$), UnaryOp("isPositive", Locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	| precision3 GREATEROREQUAL precision3
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), UnaryOp("not", 
			Locic::AST::makeNode(LOC(&@$), UnaryOp("isNegative", Locic::AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	;
	
precision1:
	precision2
	{
		$$ = $1;
	}
	| precision2 QUESTIONMARK precision1 COLON precision1
	{
		$$ = MAKESYM(Locic::AST::makeNode(LOC(&@$), Locic::AST::Value::Ternary(GETSYM($1), GETSYM($3), GETSYM($5))));
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

int Locic_Parser_GeneratedParser_lex(Locic::Parser::Token * token, Locic::Parser::LocationInfo* locationInfo, void * lexer, Locic::Parser::Context * parserContext){
	return Locic::Parser::LexGetToken(token, locationInfo, lexer);
}

int Locic_Parser_GeneratedParser_error(Locic::Parser::LocationInfo* locationInfo, void * scanner, Locic::Parser::Context * parserContext, const char *s) {
	parserContext->error(s, LOC(locationInfo));
	return 0;
}

