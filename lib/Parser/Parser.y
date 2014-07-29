/* Parser */

%{

#include <assert.h>
#include <stdio.h>

#include <list>
#include <string>
#include <vector>

#include <locic/AST.hpp>
#include <locic/Name.hpp>
#include <locic/Version.hpp>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Parser/Context.hpp>

#include "Lexer.hpp"
#include "LocationInfo.hpp"
#include "Token.hpp"

int Locic_Parser_GeneratedParser_error(locic::Parser::LocationInfo* locationInfo, void * scanner, locic::Parser::Context * parserContext, const char *s);
int Locic_Parser_GeneratedParser_lex(locic::Parser::Token * token, locic::Parser::LocationInfo* locationInfo, void * lexer, locic::Parser::Context * parserContext);

static locic::Debug::SourceLocation convertLocationInfo(const std::string& fileName, const locic::Parser::LocationInfo* locationInfo) {
	return locic::Debug::SourceLocation(fileName,
		locic::Debug::SourceRange(
			locic::Debug::SourcePosition(locationInfo->first_line, locationInfo->first_column),
			locic::Debug::SourcePosition(locationInfo->last_line, locationInfo->last_column)
		),
		std::make_pair(locationInfo->first_byte, locationInfo->last_byte),
		std::make_pair(locationInfo->first_line_byte, locationInfo->last_line_byte)
	);
}

static std::string readString(const std::string& fileName, const locic::Parser::LocationInfo* locationInfo) {
	// TODO: this needs a lot of improvement, and should probably
	//       be moved out of here entirely.
	const auto handle = fopen(fileName.c_str(), "rb");
	
	const auto length = locationInfo->last_byte - locationInfo->first_byte;
	
	std::vector<char> data;
	data.resize(length + 1);
	fseek(handle, locationInfo->first_byte, SEEK_SET);
	(void) fread(data.data(), 1, length, handle);
	data.at(length) = '\0';
	
	fclose(handle);
	
	return std::string(data.data());
}

#define READ(locationInfo) (readString(parserContext->fileName(), (locationInfo)))

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
%expect 8
%expect-rr 3

%lex-param {void * scanner}
%lex-param {locic::Parser::Context * parserContext}
%parse-param {void * scanner}
%parse-param {locic::Parser::Context * parserContext}

%union{
	// Lexer.
	std::string* lexer_str;
	locic::Constant* lexer_constant;
	locic::Version* lexer_version;
	
	// Names.
	std::string* str;
	locic::AST::Node<std::string>* string;
	locic::AST::Node<locic::AST::StringList>* stringList;
	
	locic::AST::Node<locic::Version>* version;
	
	// Signed modifier.
	locic::AST::Type::SignedModifier signedModifier;
	
	// Boolean values.
	bool boolVal;
	
	// Constants.
	locic::AST::Node<locic::Constant>* constant;
	
	// Structures.
	locic::AST::Node<locic::AST::NamespaceData>* namespaceData;
	locic::AST::Node<locic::AST::Namespace>* nameSpace;
	locic::AST::Node<locic::AST::TypeInstance>* typeInstance;
	locic::AST::Node<locic::AST::TypeInstanceList>* typeInstanceList;
	locic::AST::Node<locic::AST::Function>* function;
	locic::AST::Node<locic::AST::FunctionList>* functionList;
	locic::AST::Node<locic::AST::ModuleScope>* moduleScope;
	locic::AST::Node<locic::AST::TypeAlias>* typeAlias;
	
	// Exception initializer.
	locic::AST::Node<locic::AST::ExceptionInitializer>* exceptionInitializer;
	
	// Symbol names.
	locic::AST::Node<locic::AST::SymbolElement>* symbolElement;
	locic::AST::Node<locic::AST::Symbol>* symbol;
	
	// Type information.
	locic::AST::Node<locic::AST::Type>* type;
	locic::AST::Node<locic::AST::TypeList>* typeList;
	locic::AST::Node<locic::AST::TypeVar>* typeVar;
	locic::AST::Node<locic::AST::TypeVarList>* typeVarList;
	locic::AST::Node<locic::AST::TemplateTypeVar>* templateTypeVar;
	locic::AST::Node<locic::AST::TemplateTypeVarList>* templateTypeVarList;
	
	// Catch clause.
	locic::AST::Node<locic::AST::CatchClause>* catchClause;
	locic::AST::Node<locic::AST::CatchClauseList>* catchClauseList;
	
	// Switch case.
	locic::AST::Node<locic::AST::SwitchCase>* switchCase;
	locic::AST::Node<locic::AST::SwitchCaseList>* switchCaseList;
	
	// If clause.
	locic::AST::Node<locic::AST::IfClause>* ifClause;
	locic::AST::Node<locic::AST::IfClauseList>* ifClauseList;
	
	// Program code.
	locic::AST::Node<locic::AST::Scope>* scope;
	locic::AST::Node<locic::AST::Statement>* statement;
	locic::AST::Node<locic::AST::StatementList>* statementList;
	
	// Values.
	locic::AST::Value::CastKind castKind;
	locic::AST::Node<locic::AST::Value>* value;
	locic::AST::Node<locic::AST::ValueList>* valueList;
}

// ================ Terminals ================
%token <lexer_str> NAME
%token <lexer_version> VERSION
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
%token LROUNDBRACKET
%token RROUNDBRACKET
%token LTRIBRACKET
%token RTRIBRACKET
%token DOUBLE_LTRIBRACKET
%token DOUBLE_RTRIBRACKET

%token AUTO
%token STATIC
%token IMPORT
%token EXPORT
%token MOVE
%token LVAL
%token NOLVAL
%token REF
%token STATICREF
%token NOREF
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
%token SCOPE
%token NOEXCEPT

%token SIZEOF
%token TYPEOF
%token TYPEID

%token PRIMITIVE
%token STRUCT
%token CLASS
%token DATATYPE

%token SIGNED
%token UNSIGNED
%token BYTE
%token SHORT
%token INT
%token LONG
%token FLOAT
%token DOUBLE

%token COLON
%token VOID
%token FINAL
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
%token ASSERT
%token UNREACHABLE
%token AT
%token NULLVAL

%token CAST
%token CONST_CAST
%token STATIC_CAST
%token DYNAMIC_CAST
%token REINTERPRET_CAST

%token IS_A
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

%token SELF
%token THIS

// ================ Non-Terminals ================
%type <nameSpace> rootNamespace
%type <namespaceData> namespaceData
%type <nameSpace> nameSpace

%type <typeAlias> nonTemplatedTypeAlias
%type <typeAlias> typeAlias

%type <string> moduleNameComponent
%type <stringList> moduleName
%type <version> moduleVersion
%type <moduleScope> moduleScope

%type <typeInstance> unionDatatypeEntry
%type <typeInstanceList> unionDatatypeEntryList

%type <exceptionInitializer> exceptionInitializer

%type <typeInstance> typeInstance
%type <typeInstance> nonTemplatedTypeInstance

%type <str> functionName

%type <function> nonTemplatedFunctionDecl
%type <function> nonTemplatedFunctionDef
%type <function> functionDecl
%type <function> functionDef

%type <boolVal> noexceptSpecifier
%type <boolVal> constSpecifier

%type <type> staticMethodReturn

%type <function> nonTemplatedStaticMethodDecl
%type <function> nonTemplatedStaticMethodDef
%type <function> nonTemplatedMethodDecl
%type <function> nonTemplatedMethodDef
%type <function> methodDecl
%type <function> methodDef
%type <functionList> methodDeclList
%type <functionList> methodDefList

%type <type> objectType
%type <type> constModifiedObjectType
%type <type> pointerType
%type <signedModifier> signedModifier
%type <signedModifier> optionalSignedModifier
%type <type> integerType
%type <type> floatType

%type <type> typePrecision4
%type <type> typePrecision3
%type <type> typePrecision2
%type <type> typePrecision1
%type <type> typePrecision0
%type <type> type

%type <typeList> nonEmptyTypeList
%type <typeList> typeList
%type <typeVar> patternTypeVar
%type <typeVar> typeVar
%type <typeVarList> nonEmptyTypeVarList
%type <typeVarList> typeVarList
%type <typeVarList> structVarList
%type <templateTypeVar> templateTypeVar
%type <templateTypeVarList> templateTypeVarList

%type <symbolElement> symbolElement
%type <symbol> symbol

%type <catchClause> catchClause
%type <catchClauseList> catchClauseList

%type <switchCase> switchCase
%type <switchCaseList> switchCaseList

%type <ifClause> ifClause
%type <ifClauseList> ifClauseList
%type <statement> ifStatement

%type <scope> scope
%type <statementList> statementList
%type <statement> scopedStatement
%type <statement> normalStatement

%type <constant> constant

%type <castKind> castKind
%type <value> value
%type <valueList> nonEmptyValueList
%type <valueList> valueList
%type <value> precision0
%type <value> precision1
%type <value> precision2
%type <value> precision3
%type <value> precision4
%type <value> precision5
%type <value> precision6
%type <value> precision7
%type <value> precision8

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
	| namespaceData IMPORT functionDecl
	{
		(GETSYM($1))->functions.push_back(GETSYM($3));
		GETSYM($3)->setImport();
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData functionDef
	{
		(GETSYM($1))->functions.push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData EXPORT functionDef
	{
		(GETSYM($1))->functions.push_back(GETSYM($3));
		GETSYM($3)->setExport();
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData typeAlias
	{
		(GETSYM($1))->typeAliases.push_back(GETSYM($2));
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
	| namespaceData moduleScope
	{
		(GETSYM($1))->moduleScopes.push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| namespaceData error
	{
		parserContext->error("Invalid struct, class, function or other.", LOC(&@2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

moduleNameComponent:
	NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), $1));
	}
	;

moduleName:
	moduleNameComponent
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::StringList(1, GETSYM($1))));
	}
	| moduleName DOT moduleNameComponent
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

moduleVersion:
	VERSION
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), $1));
	}
	;

moduleScope:
	IMPORT LCURLYBRACKET namespaceData RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ModuleScope::Import(GETSYM($3))));
	}
	| EXPORT LCURLYBRACKET namespaceData RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ModuleScope::Export(GETSYM($3))));
	}
	| IMPORT moduleName moduleVersion LCURLYBRACKET namespaceData RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ModuleScope::NamedImport(GETSYM($2), GETSYM($3), GETSYM($5))));
	}
	| EXPORT moduleName moduleVersion LCURLYBRACKET namespaceData RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ModuleScope::NamedExport(GETSYM($2), GETSYM($3), GETSYM($5))));
	}
	;

nameSpace:
	NAMESPACE NAME LCURLYBRACKET namespaceData RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Namespace(GETSYM($2), GETSYM($4))));
	}
	;

nonTemplatedTypeAlias:
	USING NAME SETEQUAL type SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeAlias(GETSYM($2), GETSYM($4))));
	}
	;

typeAlias:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET nonTemplatedTypeAlias
	{
		(GETSYM($5))->templateVariables = GETSYM($3);
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($5)).get()));
	}
	| nonTemplatedTypeAlias
	{
		$$ = $1;
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
	| NULLVAL
	{
		$$ = MAKESYM(std::string("null"));
	}
	;

noexceptSpecifier:
	/* empty */
	{
		$$ = false;
	}
	| NOEXCEPT
	{
		$$ = true;
	}
	;

nonTemplatedFunctionDecl:
	type functionName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier SEMICOLON
	{
		const bool isVarArg = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, $6, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET noexceptSpecifier SEMICOLON
	{
		const bool isVarArg = true;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, $9, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@6));
		const bool isVarArg = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, false, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@9));
		const bool isVarArg = true;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, false, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	;

functionDecl:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET nonTemplatedFunctionDecl
	{
		(GETSYM($5))->setTemplateVariables(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($5)).get()));
	}
	| nonTemplatedFunctionDecl
	{
		$$ = $1;
	}
	;

nonTemplatedFunctionDef:
	type functionName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Def($6, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($7))));
	}
	;

functionDef:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET nonTemplatedFunctionDef
	{
		(GETSYM($5))->setTemplateVariables(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($5)).get()));
	}
	| nonTemplatedFunctionDef
	{
		$$ = $1;
	}
	;

constSpecifier:
	/* empty */
	{
		$$ = false;
	}
	| CONST
	{
		$$ = true;
	}
	;

nonTemplatedStaticMethodDecl:
	STATIC type functionName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticMethodDecl($7, GETSYM($2), GETSYM($3), GETSYM($5))));
	}
	;
	
nonTemplatedMethodDecl:
	nonTemplatedStaticMethodDecl
	{
		$$ = $1;
	}
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET constSpecifier noexceptSpecifier SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::MethodDecl($6, $7, GETSYM($1), GETSYM($2), GETSYM($4))));
	}
	;

methodDecl:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET nonTemplatedMethodDecl
	{
		(GETSYM($5))->setTemplateVariables(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($5)).get()));
	}
	| nonTemplatedMethodDecl
	{
		$$ = $1;
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
	
nonTemplatedStaticMethodDef:
	STATIC functionName SETEQUAL DEFAULT SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::DefaultStaticMethodDef(GETSYM($2))));
	}
	| STATIC staticMethodReturn functionName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticMethodDef($7, GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($8))));
	}
	;
	
nonTemplatedMethodDef:
	nonTemplatedStaticMethodDef
	{
		$$ = $1;
	}
	| TILDA scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Destructor(GETSYM($2))));
	}
	| functionName SETEQUAL DEFAULT SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::DefaultMethodDef(GETSYM($1))));
	}
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET constSpecifier noexceptSpecifier scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::MethodDef($6, $7, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($8))));
	}
	;

methodDef:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET nonTemplatedMethodDef
	{
		(GETSYM($5))->setTemplateVariables(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($5)).get()));
	}
	| nonTemplatedMethodDef
	{
		$$ = $1;
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

unionDatatypeEntry:
	NAME LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Datatype(GETSYM($1), GETSYM($3))));
	}
	;

unionDatatypeEntryList:
	unionDatatypeEntry
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeInstanceList(1, GETSYM($1))));
	}
	| unionDatatypeEntryList VERTICAL_BAR unionDatatypeEntry
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
exceptionInitializer:
	/* empty */
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ExceptionInitializer::None()));
	}
	| COLON symbol LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ExceptionInitializer::Initialize(GETSYM($2), GETSYM($4))));
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
	| DATATYPE NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Datatype(GETSYM($2), GETSYM($4))));
	}
	| DATATYPE NAME SETEQUAL unionDatatypeEntryList SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::UnionDatatype(GETSYM($2), GETSYM($4))));
	}
	| EXCEPTION NAME LROUNDBRACKET typeVarList RROUNDBRACKET exceptionInitializer SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Exception(GETSYM($2), GETSYM($4), GETSYM($6))));
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

objectType:
	symbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Object(GETSYM($1))));
	}
	;

constModifiedObjectType:
	objectType
	{
		$$ = $1;
	}
	| CONST objectType
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Const(GETSYM($2))));
	}
	;

signedModifier:
	SIGNED
	{
		$$ = locic::AST::Type::SIGNED;
	}
	| UNSIGNED
	{
		$$ = locic::AST::Type::UNSIGNED;
	}
	;

optionalSignedModifier:
	/* empty */
	{
		$$ = locic::AST::Type::NO_SIGNED;
	}
	| signedModifier
	{
		$$ = $1;
	}
	;

optionalInt:
	/* empty */
	| INT
	;

integerType:
	optionalSignedModifier BYTE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, "byte")));
	}
	| optionalSignedModifier SHORT optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, "short")));
	}
	| optionalSignedModifier INT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, "int")));
	}
	| SIGNED
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::SIGNED, "int")));
	}
	| UNSIGNED
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::UNSIGNED, "int")));
	}
	
	// Handle ridiculous numbers of shift/reduce conflicts with
	// 'long double' by splitting between cases where signed
	// is and isn't specified (so that 'LONG' always means shift).
	| LONG optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::NO_SIGNED, "long")));
	}
	| signedModifier LONG optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, "long")));
	}
	| LONG LONG optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::NO_SIGNED, "longlong")));
	}
	| signedModifier LONG LONG optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, "longlong")));
	}
	;

floatType:
	FLOAT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Float("float")));
	}
	| DOUBLE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Float("double")));
	}
	| LONG DOUBLE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Float("longdouble")));
	}
	;

typePrecision4:
	VOID
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Void()));
	}
	| AUTO
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Auto()));
	}
	| integerType
	{
		$$ = $1;
	}
	| floatType
	{
		$$ = $1;
	}
	| objectType
	{
		$$ = $1;
	}
	| LROUNDBRACKET typePrecision1 RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Bracket(GETSYM($2))));
	}
	| LROUNDBRACKET STAR RROUNDBRACKET LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET typeList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Function(GETSYM($5), GETSYM($8))));
	}
	| LROUNDBRACKET STAR RROUNDBRACKET LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET nonEmptyTypeList COMMA DOT DOT DOT RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::VarArgFunction(GETSYM($5), GETSYM($8))));
	}
	| LROUNDBRACKET error RROUNDBRACKET
	{
		parserContext->error("Invalid type.", LOC(&@2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Auto()));
	}
	;

typePrecision3:
	typePrecision4
	{
		$$ = $1;
	}
	| CONST typePrecision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Const(GETSYM($2))));
	}
	;

typePrecision2:
	typePrecision3
	{
		$$ = $1;
	}
	| LVAL LTRIBRACKET type RTRIBRACKET typePrecision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Lval(GETSYM($3), GETSYM($5))));
	}
	| REF LTRIBRACKET type RTRIBRACKET typePrecision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Ref(GETSYM($3), GETSYM($5))));
	}
	| STATICREF LTRIBRACKET type RTRIBRACKET typePrecision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::StaticRef(GETSYM($3), GETSYM($5))));
	}
	;

pointerType:
	typePrecision1 STAR
	{
		// Create 'ptr<TYPE>'.
		auto typeList = locic::AST::makeNode(LOC(&@1), new locic::AST::TypeList(1, GETSYM($1)));
		auto symbolElement = locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement("__ptr", typeList));
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

patternTypeVar:
	constModifiedObjectType LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeVar::PatternVar(GETSYM($1), GETSYM($3))));
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
	| patternTypeVar
	{
		$$ = $1;
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
	
switchCase:
	CASE patternTypeVar scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::SwitchCase(GETSYM($2), GETSYM($3))));
	}
	;
	
switchCaseList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::SwitchCaseList()));
	}
	| switchCaseList switchCase
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

catchClause:
	CATCH LROUNDBRACKET typeVar RROUNDBRACKET scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::CatchClause(GETSYM($3), GETSYM($5))));
	}
	;

catchClauseList:
	catchClause
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::CatchClauseList(1, GETSYM($1))));
	}
	| catchClauseList catchClause
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
ifClause:
	IF LROUNDBRACKET value RROUNDBRACKET scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::IfClause(GETSYM($3), GETSYM($5))));
	}
	;
	
ifClauseList:
	ifClause
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::IfClauseList(1, GETSYM($1))));
	}
	| ifClauseList ELSE ifClause
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;
	
ifStatement:
	ifClauseList
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::If(GETSYM($1), locic::AST::makeNode(LOC(&@1), new locic::AST::Scope()))));
	}
	| ifClauseList ELSE scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::If(GETSYM($1), GETSYM($3))));
	}
	;
	
scopedStatement:
	scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ScopeStmt(GETSYM($1))));
	}
	| ifStatement
	{
		$$ = $1;
	}
	| SWITCH LROUNDBRACKET value RROUNDBRACKET LCURLYBRACKET switchCaseList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Switch(GETSYM($3), GETSYM($6))));
	}
	| FOR LROUNDBRACKET typeVar COLON value RROUNDBRACKET scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::For(GETSYM($3), GETSYM($5), GETSYM($7))));
	}
	| WHILE LROUNDBRACKET value RROUNDBRACKET scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::While(GETSYM($3), GETSYM($5))));
	}
	| TRY scope catchClauseList
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Try(GETSYM($2), GETSYM($3))));
	}
	| SCOPE LROUNDBRACKET NAME RROUNDBRACKET scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ScopeExit(GETSYM($3), GETSYM($5))));
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
	| precision5 PERCENTEQUAL precision6
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
	| THROW value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Throw(GETSYM($2))));
	}
	| THROW
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Rethrow()));
	}
	| CONTINUE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Continue()));
	}
	| BREAK
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Break()));
	}
	| ASSERT value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Assert(GETSYM($2), READ(&@2))));
	}
	| UNREACHABLE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Unreachable()));
	}
	;

constant:
	CONSTANT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), $1));
	}
	| NULLVAL
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::Constant::Null()));
	}
	;

castKind:
	CAST
	{
		$$ = locic::AST::Value::CAST_STATIC;
	}
	| STATIC_CAST
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
	
precision8:
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
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Literal("", GETSYM($1))));
	}
	| NAME constant
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Literal(GETSYM($1), GETSYM($2))));
	}
	| constant NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Literal(GETSYM($2), GETSYM($1))));
	}
	| castKind LTRIBRACKET type COMMA type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Cast($1, GETSYM($3), GETSYM($5), GETSYM($8))));
	}
	| LVAL LTRIBRACKET type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Lval(GETSYM($3), GETSYM($6))));
	}
	| NOLVAL LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::NoLval(GETSYM($3))));
	}
	| REF LTRIBRACKET type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Ref(GETSYM($3), GETSYM($6))));
	}
	| NOREF LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::NoRef(GETSYM($3))));
	}
	| SELF
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Self()));
	}
	| THIS
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::This()));
	}
	| SIZEOF LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::SizeOf(GETSYM($3))));
	}
	;
	
precision7:
	precision8
	{
		$$ = $1;
	}
	| precision7 DOT NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::MemberAccess(GETSYM($1), GETSYM($3))));
	}
	| precision7 DOT NAME LTRIBRACKET nonEmptyTypeList RTRIBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::TemplatedMemberAccess(GETSYM($1), GETSYM($3), GETSYM($5))));
	}
	| precision7 PTRACCESS NAME
	{
		const auto derefNode = locic::AST::makeNode(LOC(&@$), UnaryOp("deref", GETSYM($1)));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::MemberAccess(derefNode, GETSYM($3))));
	}
	| precision7 LROUNDBRACKET valueList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::FunctionCall(GETSYM($1), GETSYM($3))));
	}
	| precision7 LSQUAREBRACKET value RSQUAREBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("index", GETSYM($1), GETSYM($3))));
	}
	;
	
precision6:
	precision7
	{
		$$ = $1;
	}
	| PLUS precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("plus", GETSYM($2))));
	}
	| MINUS precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("minus", GETSYM($2))));
	}
	| EXCLAIMMARK precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("not", GETSYM($2))));
	}
	| AMPERSAND precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("address", GETSYM($2))));
	}
	| STAR precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("deref", GETSYM($2))));
	}
	| MOVE precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), UnaryOp("move", GETSYM($2))));
	}
	;
	
precision5:
	precision6
	{
		$$ = $1;
	}
	| precision5 STAR precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("multiply", GETSYM($1), GETSYM($3))));
	}
	| precision5 FORWARDSLASH precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("divide", GETSYM($1), GETSYM($3))));
	}
	| precision5 PERCENT precision6
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("modulo", GETSYM($1), GETSYM($3))));
	}
	;
	
precision4:
	precision5
	{
		$$ = $1;
	}
	| precision4 PLUS precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("add", GETSYM($1), GETSYM($3))));
	}
	| precision4 MINUS precision5
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp("subtract", GETSYM($1), GETSYM($3))));
	}
	;
	
precision3:
	precision4
	{
		$$ = $1;
	}
	| precision4 ISEQUAL precision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_ISEQUAL, GETSYM($1), GETSYM($3))));
	}
	| precision4 NOTEQUAL precision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_NOTEQUAL, GETSYM($1), GETSYM($3))));
	}
	| precision4 LTRIBRACKET precision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LESSTHAN, GETSYM($1), GETSYM($3))));
	}
	| precision4 RTRIBRACKET precision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_GREATERTHAN, GETSYM($1), GETSYM($3))));
	}
	| precision4 LESSOREQUAL precision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LESSTHANOREQUAL, GETSYM($1), GETSYM($3))));
	}
	| precision4 GREATEROREQUAL precision4
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_GREATERTHANOREQUAL, GETSYM($1), GETSYM($3))));
	}
	;

precision2:
	precision3
	{
		$$ = $1;
	}
	| precision2 DOUBLE_AMPERSAND precision3
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LOGICALAND, GETSYM($1), GETSYM($3))));
	}
	;

precision1:
	precision2
	{
		$$ = $1;
	}
	| precision1 DOUBLE_VERTICAL_BAR precision2
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LOGICALOR, GETSYM($1), GETSYM($3))));
	}
	;
	
precision0:
	precision1
	{
		$$ = $1;
	}
	| precision1 QUESTIONMARK precision0 COLON precision0
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Ternary(GETSYM($1), GETSYM($3), GETSYM($5))));
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

