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

static AST::Value* UnaryOp(const std::string& name, AST::Node<AST::Value> operand) {
	const auto paramNode = AST::Node<AST::ValueList>(operand.location(), new AST::ValueList());
	return AST::Value::FunctionCall(AST::makeNode(operand.location(), AST::Value::MemberAccess(operand, name)), paramNode);
}

static AST::Value* BinaryOp(const std::string& name, AST::Node<AST::Value> leftOperand, AST::Node<AST::Value> rightOperand) {
	const auto paramNode = AST::makeNode(rightOperand.location(), new AST::ValueList(1, rightOperand));
	return AST::Value::FunctionCall(AST::makeNode(leftOperand.location(), AST::Value::MemberAccess(leftOperand, name)), paramNode);
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
	AST::Node<Locic::Constant>* constant;
	
	// Structures.
	AST::Node<AST::NamespaceData>* namespaceData;
	AST::Node<AST::Namespace>* nameSpace;
	AST::Node<AST::TypeInstance>* typeInstance;
	AST::Node<AST::Function>* function;
	AST::Node<AST::FunctionList>* functionArray;
	
	// Symbol names.
	AST::Node<AST::SymbolElement>* symbolElement;
	AST::Node<AST::Symbol>* symbol;
	
	// Type information.
	AST::Node<AST::Type>* type;
	AST::Node<AST::TypeList>* typeArray;
	AST::Node<AST::TypeVar>* typeVar;
	AST::Node<AST::TypeVarList>* typeVarArray;
	AST::Node<AST::TemplateTypeVar>* templateTypeVar;
	AST::Node<AST::TemplateTypeVarList>* templateTypeVarArray;
	
	// Program code.
	AST::Node<AST::Scope>* scope;
	AST::Node<AST::Statement>* statement;
	AST::Node<AST::StatementList>* statementArray;
	
	// Values.
	AST::Value::CastKind castKind;
	AST::Node<AST::Value>* value;
	AST::Node<AST::ValueList>* valueArray;
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::Namespace("", GETSYM($1))));
	}
	;

namespaceData:
	// empty
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::NamespaceData()));
	}
	| namespaceData SEMICOLON
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData functionDecl
	{
		(GETSYM($1))->functions.push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData functionDef
	{
		(GETSYM($1))->functions.push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData typeInstance
	{
		(GETSYM($1))->typeInstances.push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData nameSpace
	{
		(GETSYM($1))->namespaces.push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData error
	{
		parserContext->error("Invalid struct, class, function or other.", LOC(&@2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

nameSpace:
	NAMESPACE NAME LCURLYBRACKET namespaceData RCURLYBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::Namespace(GETSYM($2), GETSYM($4))));
	}
	;

structVarList:
	// empty
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::TypeVarList()));
	}
	| structVarList typeVar SEMICOLON
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| structVarList SEMICOLON
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

staticFunctionDecl:
	STATIC NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		const auto implicitAutoType = AST::makeNode(LOC(&@1), AST::Type::Undefined());
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::Decl(implicitAutoType, GETSYM($2), GETSYM($4))));
	}
	| STATIC type NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::Decl(GETSYM($2), GETSYM($3), GETSYM($5))));
	}
	;

staticFunctionDef:
	STATIC NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		const auto implicitAutoType = AST::makeNode(LOC(&@1), AST::Type::Undefined());
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::Def(implicitAutoType, GETSYM($2), GETSYM($4), GETSYM($6))));
	}
	| STATIC type NAME LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::Def(GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($7))));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::Decl(GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET SEMICOLON
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::VarArgDecl(GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@6));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::Decl(GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@9));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::VarArgDecl(GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	;
	
functionDef:
	type functionName LROUNDBRACKET typeVarList RROUNDBRACKET scope
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::Def(GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($6))));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Function::Destructor(GETSYM($2))));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::FunctionList()));
	}
	| classFunctionDeclList classFunctionDecl
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
classFunctionDefList:
	// empty
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::FunctionList()));
	}
	| classFunctionDefList classFunctionDef
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

templateTypeVar:
	TYPENAME NAME
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TemplateTypeVar::Typename(GETSYM($2))));
	}
	| TYPENAME NAME COLON type
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TemplateTypeVar::TypenameSpec(GETSYM($2), GETSYM($4))));
	}
	;

templateTypeVarList:
	templateTypeVar
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::TemplateTypeVarList(1, GETSYM($1))));
	}
	| templateTypeVarList COMMA templateTypeVar
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

typeInstance:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET nonTemplatedTypeInstance
	{
		(GETSYM($5))->templateVariables = GETSYM($3);
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($5)).get()));
	}
	| nonTemplatedTypeInstance
	{
		$$ = $1;
	}
	;

nonTemplatedTypeInstance:
	STRUCT NAME LCURLYBRACKET structVarList RCURLYBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeInstance::Struct(GETSYM($2), GETSYM($4))));
	}
	| CLASS NAME LCURLYBRACKET classFunctionDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeInstance::ClassDecl(GETSYM($2), GETSYM($4))));
	}
	| CLASS NAME LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET classFunctionDefList RCURLYBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeInstance::ClassDef(GETSYM($2), GETSYM($4), GETSYM($7))));
	}
	| PRIMITIVE NAME LCURLYBRACKET classFunctionDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeInstance::Primitive(GETSYM($2), GETSYM($4))));
	}
	| INTERFACE NAME LCURLYBRACKET classFunctionDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeInstance::Interface(GETSYM($2), GETSYM($4))));
	}
	| DATATYPE NAME LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeInstance::Datatype(GETSYM($2), GETSYM($4))));
	}
	;

symbolElement:
	NAME
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::SymbolElement(GETSYM($1), AST::makeDefaultNode<AST::TypeList>())));
	}
	| NAME LTRIBRACKET nonEmptyTypeList RTRIBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::SymbolElement(GETSYM($1), GETSYM($3))));
	}
	| TYPENAME
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::SymbolElement("typename_type", AST::makeDefaultNode<AST::TypeList>())));
	}
	;
	
symbol:
	symbolElement
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::Symbol(AST::Symbol::Relative() + GETSYM($1))));
	}
	| COLON COLON symbolElement
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::Symbol(AST::Symbol::Absolute() + GETSYM($3))));
	}
	| symbol COLON COLON symbolElement
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::Symbol(*(GETSYM($1)) + GETSYM($4))));
	}
	;
	
typePrecision3:
	VOIDNAME
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Void()));
	}
	| symbol
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Object(GETSYM($1))));
	}
	| AUTO
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Undefined()));
	}
	| LROUNDBRACKET typePrecision1 RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Bracket(GETSYM($2))));
	}
	| STAR LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET typeList RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Function(GETSYM($3), GETSYM($6))));
	}
	| STAR LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET nonEmptyTypeList COMMA DOT DOT DOT RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::VarArgFunction(GETSYM($3), GETSYM($6))));
	}
	| LROUNDBRACKET error RROUNDBRACKET
	{
		parserContext->error("Invalid type.", LOC(&@2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Undefined()));
	}
	;

typePrecision2:
	typePrecision3
	{
		$$ = $1;
	}
	| CONST typePrecision3
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Const(GETSYM($2))));
	}
	;

pointerType:
	typePrecision1 STAR
	{
		// Create 'ptr<TYPE>'.
		auto typeList = AST::makeNode(LOC(&@1), new AST::TypeList(1, GETSYM($1)));
		auto symbolElement = AST::makeNode(LOC(&@$), new AST::SymbolElement("ptr", typeList));
		auto symbol = AST::makeNode(LOC(&@$), new AST::Symbol(AST::Symbol::Absolute() + symbolElement));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Object(symbol)));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Const(GETSYM($1))));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Type::Reference(GETSYM($1))));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::TypeList(1, GETSYM($1))));
	}
	| nonEmptyTypeList COMMA type
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
typeList:
	// empty
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::TypeList()));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeVar::NamedVar(GETSYM($1), GETSYM($2), usesCustomLval)));
	}
	| LVAL type NAME
	{
		const bool usesCustomLval = true;
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeVar::NamedVar(GETSYM($2), GETSYM($3), usesCustomLval)));
	}
	| type LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeVar::PatternVar(GETSYM($1), GETSYM($3))));
	}
	| UNDERSCORE
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::TypeVar::Any()));
	}
	;
	
typeVarList:
	// empty
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::TypeVarList()));
	}
	| nonEmptyTypeVarList typeVar
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
nonEmptyTypeVarList:
	// empty
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::TypeVarList()));
	}
	| nonEmptyTypeVarList typeVar COMMA
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
valueList:
	// empty
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::ValueList()));
	}
	| nonEmptyValueList
	{
		$$ = $1;
	}
	;
	
nonEmptyValueList:
	value
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::ValueList(1, GETSYM($1))));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::Scope(GETSYM($2))));
	}
	;
	
statementList:
	// empty
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), new AST::StatementList()));
	}
	| statementList scopedStatement
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList normalStatement SEMICOLON
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList normalStatement error
	{
		parserContext->error("Statement must be terminated with semicolon.", LOC(&@3));
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList SEMICOLON
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| statementList error
	{
		parserContext->error("Invalid statement.", LOC(&@2));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
scopedStatement:
	scope
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ScopeStmt(GETSYM($1))));
	}
	| IF LROUNDBRACKET value RROUNDBRACKET scope
	{
		// One sided if statement (i.e. nothing happens in 'else' case).
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::If(GETSYM($3), GETSYM($5), AST::makeNode(LOC(&@1), new AST::Scope()))));
	}
	| IF LROUNDBRACKET value RROUNDBRACKET scope ELSE scope
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::If(GETSYM($3), GETSYM($5), GETSYM($7))));
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
		const auto emptyTypeList = AST::makeNode(LOC(&@$), new AST::TypeList());
		const auto anonVariableSymbolElement = AST::makeNode(LOC(&@$), new AST::SymbolElement(anonVariableName, emptyTypeList));
		const auto anonVariableSymbol = AST::makeNode(LOC(&@$), new AST::Symbol(AST::Symbol::Relative() + anonVariableSymbolElement));
		
		// {
		std::vector< AST::Node<AST::Statement> > statements;
		
		//     auto anon_var = range_value;
		const bool usesCustomLval = false;
		AST::Node<AST::TypeVar> anonVarTypeVar = AST::makeNode(LOC(&@$), AST::TypeVar::NamedVar(AST::makeNode(LOC(&@$), AST::Type::Undefined()), anonVariableName, usesCustomLval));
		statements.push_back(AST::makeNode(LOC(&@$), AST::Statement::VarDecl(anonVarTypeVar, GETSYM($5))));
		
		//     while (!anon_var.empty()) {
		AST::Node<AST::Value> condition = AST::makeNode(LOC(&@$), UnaryOp("not", AST::makeNode(LOC(&@$), UnaryOp("empty", AST::makeNode(LOC(&@$), AST::Value::SymbolRef(anonVariableSymbol))))));
		std::vector< AST::Node<AST::Statement> > whileLoopStatements;
		
		//         type value_var = anon_var.front();
		whileLoopStatements.push_back(AST::makeNode(LOC(&@$), AST::Statement::VarDecl(GETSYM($3), AST::makeNode(LOC(&@$), UnaryOp("front", AST::makeNode(LOC(&@$), AST::Value::SymbolRef(anonVariableSymbol)))))));
		
		//         {
		//             //...
		//         }
		whileLoopStatements.push_back(AST::makeNode(LOC(&@$), AST::Statement::ScopeStmt(GETSYM($7))));
		
		//         anon_var.popFront();
		const auto anonVariableSymbolRef = AST::makeNode(LOC(&@$), AST::Value::SymbolRef(anonVariableSymbol));
		const auto popFrontValue = AST::makeNode(LOC(&@$), UnaryOp("popFront", anonVariableSymbolRef));
		whileLoopStatements.push_back(AST::makeNode(LOC(&@$), AST::Statement::ValueStmt(popFrontValue)));
		
		//     }
		const auto whileLoopStatementsNode = AST::makeNode(LOC(&@$), new AST::StatementList(whileLoopStatements));
		const auto whileScopeNode = AST::makeNode(LOC(&@$), new AST::Scope(whileLoopStatementsNode));
		statements.push_back(AST::makeNode(LOC(&@$), AST::Statement::While(condition, whileScopeNode)));
		
		// }
		const auto statementsNode = AST::makeNode(LOC(&@$), new AST::StatementList(statements));
		const auto scopeNode = AST::makeNode(LOC(&@$), new AST::Scope(statementsNode));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ScopeStmt(scopeNode)));
	}
	| WHILE LROUNDBRACKET value RROUNDBRACKET scope
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::While(GETSYM($3), GETSYM($5))));
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
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::VarDecl(GETSYM($1), GETSYM($3))));
	}
	
	| value SETEQUAL value %dprec 1
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ValueStmt(AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), GETSYM($3))))));
	}
	
	| value ADDEQUAL value
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ValueStmt(AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), AST::makeNode(LOC(&@$), BinaryOp("add", GETSYM($1), GETSYM($3))))))));
	}
	| value SUBEQUAL value
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ValueStmt(AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), AST::makeNode(LOC(&@$), BinaryOp("subtract", GETSYM($1), GETSYM($3))))))));
	}
	| value MULEQUAL value
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ValueStmt(AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), AST::makeNode(LOC(&@$), BinaryOp("multiply", GETSYM($1), GETSYM($3))))))));
	}
	| value DIVEQUAL value
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ValueStmt(AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), AST::makeNode(LOC(&@$), BinaryOp("divide", GETSYM($1), GETSYM($3))))))));
	}
	| precision4 PERCENTEQUAL precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ValueStmt(AST::makeNode(LOC(&@$), BinaryOp("assign", GETSYM($1), AST::makeNode(LOC(&@$), BinaryOp("modulo", GETSYM($1), GETSYM($3))))))));
	}
	| value
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ValueStmt(GETSYM($1))));
	}
	| RETURN
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::ReturnVoid()));
	}
	| RETURN value
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Statement::Return(GETSYM($2))));
	}
	;

constant:
	CONSTANT
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), $1));
	}
	;

castKind:
	STATIC_CAST
	{
		$$ = AST::Value::CAST_STATIC;
	}
	| CONST_CAST
	{
		$$ = AST::Value::CAST_CONST;
	}
	| DYNAMIC_CAST
	{
		$$ = AST::Value::CAST_DYNAMIC;
	}
	| REINTERPRET_CAST
	{
		$$ = AST::Value::CAST_REINTERPRET;
	}
	;
	
precision7:
	LROUNDBRACKET precision0 RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::Bracket(GETSYM($2))));
	}
	| symbol
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::SymbolRef(GETSYM($1))));
	}
	| AT NAME
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::MemberRef(GETSYM($2))));
	}
	| AT LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::InternalConstruct(GETSYM($3))));
	}
	| constant
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::Constant(GETSYM($1))));
	}
	| castKind LTRIBRACKET type COMMA type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::Cast($1, GETSYM($3), GETSYM($5), GETSYM($8))));
	}
	;
	
precision6:
	precision7
	{
		$$ = $1;
	}
	| precision6 DOT NAME
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::MemberAccess(GETSYM($1), GETSYM($3))));
	}
	| precision6 PTRACCESS NAME
	{
		const auto derefNode = AST::makeNode(LOC(&@$), UnaryOp("deref", GETSYM($1)));
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::MemberAccess(derefNode, GETSYM($3))));
	}
	| precision6 LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::FunctionCall(GETSYM($1), GETSYM($3))));
	}
	| precision6 LSQUAREBRACKET value RSQUAREBRACKET
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), BinaryOp("index", GETSYM($1), GETSYM($3))));
	}
	;
	
precision5:
	precision6
	{
		$$ = $1;
	}
	| PLUS precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("plus", GETSYM($2))));
	}
	| MINUS precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("minus", GETSYM($2))));
	}
	| EXCLAIMMARK precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("not", GETSYM($2))));
	}
	| AMPERSAND precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("address", GETSYM($2))));
	}
	| STAR precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("deref", GETSYM($2))));
	}
	| MOVE precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("move", GETSYM($2))));
	}
	;
	
precision4:
	precision5
	{
		$$ = $1;
	}
	| precision4 STAR precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), BinaryOp("multiply", GETSYM($1), GETSYM($3))));
	}
	| precision4 FORWARDSLASH precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), BinaryOp("divide", GETSYM($1), GETSYM($3))));
	}
	| precision4 PERCENT precision5
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), BinaryOp("modulo", GETSYM($1), GETSYM($3))));
	}
	;
	
precision3:
	precision4
	{
		$$ = $1;
	}
	| precision3 PLUS precision4
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), BinaryOp("add", GETSYM($1), GETSYM($3))));
	}
	| precision3 MINUS precision4
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), BinaryOp("subtract", GETSYM($1), GETSYM($3))));
	}
	;
	
precision2:
	precision3
	{
		$$ = $1;
	}
	| precision3 ISEQUAL precision3
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("isZero", AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 NOTEQUAL precision3
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("not", 
			AST::makeNode(LOC(&@$), UnaryOp("isZero", AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	| precision3 LTRIBRACKET precision3
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("isNegative", AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 RTRIBRACKET precision3
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("isPositive", AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))));
	}
	| precision3 LESSOREQUAL precision3
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("not", 
			AST::makeNode(LOC(&@$), UnaryOp("isPositive", AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	| precision3 GREATEROREQUAL precision3
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), UnaryOp("not", 
			AST::makeNode(LOC(&@$), UnaryOp("isNegative", AST::makeNode(LOC(&@$), BinaryOp("compare", GETSYM($1), GETSYM($3))))))));
	}
	;
	
precision1:
	precision2
	{
		$$ = $1;
	}
	| precision2 QUESTIONMARK precision1 COLON precision1
	{
		$$ = MAKESYM(AST::makeNode(LOC(&@$), AST::Value::Ternary(GETSYM($1), GETSYM($3), GETSYM($5))));
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

