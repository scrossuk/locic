/* Parser */

%{

#include <assert.h>
#include <stdio.h>

#include <list>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/AST.hpp>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Parser/Context.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

#include "Lexer.hpp"
#include "LocationInfo.hpp"
#include "Token.hpp"

int Locic_Parser_GeneratedParser_error(locic::Parser::LocationInfo* locationInfo, void * scanner, locic::Parser::Context * parserContext, const char *s);
int Locic_Parser_GeneratedParser_lex(locic::Parser::Token * token, locic::Parser::LocationInfo* locationInfo, void * lexer, locic::Parser::Context * parserContext);

static locic::Debug::SourceLocation convertLocationInfo(const locic::String fileName, const locic::Parser::LocationInfo* locationInfo) {
	return locic::Debug::SourceLocation(fileName,
		locic::Debug::SourceRange(
			locic::Debug::SourcePosition(locationInfo->first_line, locationInfo->first_column),
			locic::Debug::SourcePosition(locationInfo->last_line, locationInfo->last_column)
		),
		std::make_pair(locationInfo->first_byte, locationInfo->last_byte),
		std::make_pair(locationInfo->first_line_byte, locationInfo->last_line_byte)
	);
}

static locic::String readString(const locic::Parser::Context* const parserContext, const locic::String fileName, const locic::Parser::LocationInfo* locationInfo) {
	// TODO: this needs a lot of improvement, and should probably
	//       be moved out of here entirely.
	const auto handle = fopen(fileName.c_str(), "rb");
	
	const auto length = locationInfo->last_byte - locationInfo->first_byte;
	
	std::vector<char> data;
	data.resize(length + 1);
	fseek(handle, locationInfo->first_byte, SEEK_SET);
	const size_t readSize = fread(data.data(), 1, length, handle);
	if (readSize != length) {
		throw std::runtime_error(locic::makeString("Failed to read string in file '%s'.", fileName.c_str()));
	}
	
	data.at(length) = '\0';
	
	fclose(handle);
	
	return parserContext->getString(std::string(data.data()));
}

#define READ(locationInfo) (readString(parserContext, parserContext->fileName(), (locationInfo)))

#define LOC(locationInfo) (convertLocationInfo(parserContext->fileName(), (locationInfo)))

static locic::AST::Value* UnaryOp(const locic::String& name, locic::AST::Node<locic::AST::Value> operand) {
	const auto paramNode = locic::AST::Node<locic::AST::ValueList>(operand.location(), new locic::AST::ValueList());
	return locic::AST::Value::FunctionCall(locic::AST::makeNode(operand.location(), locic::AST::Value::MemberAccess(operand, name)), paramNode);
}

static locic::AST::Value* BinaryOp(const locic::String& name, locic::AST::Node<locic::AST::Value> leftOperand, locic::AST::Node<locic::AST::Value> rightOperand) {
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

static locic::AST::Node<locic::AST::ValueList> * mergeCallArgList(LOCIC_PARSER_GENERATEDPARSER_STYPE x0,
                                                                  LOCIC_PARSER_GENERATEDPARSER_STYPE x1) {
	auto& first = *(x0.valueList);
	auto& second = *(x1.valueList);
	// Accept the call argument list that's shortest.
	return first->size() < second->size() ? &first : &second;
}

static locic::AST::Node<locic::AST::Value> * mergeValue(LOCIC_PARSER_GENERATEDPARSER_STYPE x0,
                                                        LOCIC_PARSER_GENERATEDPARSER_STYPE x1) {
	auto& first = *(x0.value);
	auto& second = *(x1.value);
	return MAKESYM(locic::AST::makeNode(first.location(), locic::AST::Value::Merge(first, second)));
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

// Shift/reduce conflicts occur due to the triangle bracket
// template syntax conflicting with the less-than operator.
// 
// For example, you can have:
// 
// Type < Arg0, Arg1 >( values... )
// 
// ...and:
// 
// Value < Value
// 
// Ultimately it's unambiguous whether the expression is
// a template type or a more-than expression but due to
// limited lookahead shift/reduce conflicts occur.
%expect 5

// Reduce-reduce conflicts occur due to:
// 
// 1. ref<>() and lval<>() - these will soon be removed.
// 2. parser conflicts between reference types and bitwise-and,
//    and between pointer types and multiply - this is fixable
//    by merging these type structures into values.
%expect-rr 5

%lex-param {void * scanner}
%lex-param {locic::Parser::Context * parserContext}
%parse-param {void * scanner}
%parse-param {locic::Parser::Context * parserContext}

%union{
	// Lexer.
	locic::String lexer_str;
	locic::Constant lexer_constant;
	locic::Version* lexer_version;
	
	// Names.
	locic::String string;
	locic::AST::Node<locic::Name>* name;
	locic::AST::Node<locic::AST::StringList>* stringList;
	
	locic::AST::Node<locic::Version>* version;
	
	// Signed modifier.
	locic::AST::Type::SignedModifier signedModifier;
	
	// Boolean values.
	bool boolVal;
	
	// Constants.
	locic::AST::Node<locic::Constant>* constant;
	
	// Structures.
	locic::AST::Node<locic::AST::AliasDecl>* alias;
	locic::AST::Node<locic::AST::NamespaceData>* namespaceData;
	locic::AST::Node<locic::AST::NamespaceDecl>* nameSpace;
	locic::AST::Node<locic::AST::TypeInstance>* typeInstance;
	locic::AST::Node<locic::AST::TypeInstanceList>* typeInstanceList;
	locic::AST::Node<locic::AST::Function>* function;
	locic::AST::Node<locic::AST::FunctionList>* functionList;
	locic::AST::Node<locic::AST::ModuleScope>* moduleScope;

	// Static Assert
	locic::AST::Node<locic::AST::StaticAssert>* staticAssert;
	
	// Exception initializer.
	locic::AST::Node<locic::AST::ExceptionInitializer>* exceptionInitializer;
	
	// Predicate.
	locic::AST::Node<locic::AST::Predicate>* predicateExpr;
	
	// Specifiers.
	locic::AST::Node<locic::AST::ConstSpecifier>* constSpecifier;
	locic::AST::Node<locic::AST::RequireSpecifier>* requireSpecifier;
	
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
	locic::AST::Node<locic::AST::DefaultCase>* defaultCase;
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

%token TRUEVAL
%token FALSEVAL

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
%token DOUBLE_LTRIBRACKET
%token LTRIBRACKET
%token RTRIBRACKET

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
%token NOTAG

%token TEMPLATE
%token TYPENAME
%token VIRTUAL
%token REQUIRE
%token UNUSED
%token UNUSED_RESULT

%token USING
%token LET
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

%token ALIGNOF
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
%token UBYTE
%token SHORT
%token USHORT
%token INT
%token UINT
%token LONG
%token ULONG
%token LONGLONG
%token ULONGLONG
%token FLOAT
%token DOUBLE

%token COLON
%token DOUBLE_COLON
%token VOID
%token FINAL
%token CONST
%token MUTABLE
%token OVERRIDE_CONST

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

%token CONST_CAST
%token DYNAMIC_CAST
%token REINTERPRET_CAST

%token AND
%token OR

%token IS_A
%token DOT
%token PTRACCESS
%token PLUS
%token DOUBLE_PLUS
%token MINUS
%token DOUBLE_MINUS
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

%type <alias> nonTemplatedAlias
%type <alias> alias

%type <staticAssert> staticAssert

%type <string> moduleNameComponent
%type <stringList> moduleName
%type <version> moduleVersion
%type <moduleScope> moduleScope

%type <typeInstance> unionDatatypeEntry
%type <typeInstanceList> unionDatatypeEntryList

%type <exceptionInitializer> exceptionInitializer

%type <typeInstance> typeInstance
%type <typeInstance> nonTemplatedTypeInstance

%type <stringList> stringList
%type <stringList> noTagSet

%type <function> nonTemplatedFunctionDecl
%type <function> nonTemplatedFunctionDef
%type <function> importedFunctionDecl
%type <function> exportedFunctionDef
%type <function> functionDecl
%type <function> functionDef

%type <predicateExpr> predicateExprAtom
%type <predicateExpr> predicateExprBinaryOperator
%type <predicateExpr> predicateExpr

%type <constSpecifier> constSpecifier
%type <requireSpecifier> moveSpecifier
%type <requireSpecifier> noexceptSpecifier
%type <requireSpecifier> requireSpecifier

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
%type <type> typeValue

%type <type> typePrecision4WithSymbol
%type <type> typePrecision3WithSymbol
%type <type> typePrecision1WithSymbol
%type <type> typePrecision0WithSymbol
%type <type> type

%type <typeVar> patternTypeVar
%type <typeVar> basicTypeVar
%type <typeVar> overrideConstTypeVar
%type <typeVar> finalTypeVar
%type <typeVar> unusedTypeVar
%type <typeVar> typeVar
%type <typeVarList> nonEmptyTypeVarList
%type <typeVarList> typeVarList
%type <typeVarList> cTypeVarList

%type <typeList> emptyTypeList
%type <typeList> nonEmptyTypeList
%type <typeList> typeList
%type <templateTypeVar> templateTypeVar
%type <templateTypeVarList> templateTypeVarList

%type <valueList> emptyTemplateArgumentList
%type <valueList> templateArgumentList
%type <valueList> templateValueList

%type <string> functionNameElement
%type <name> functionName
%type <name> methodName
%type <string> destructorNameElement
%type <name> destructorName
%type <symbolElement> symbolElement
%type <symbol> symbol

%type <catchClause> catchClause
%type <catchClauseList> catchClauseList

%type <switchCase> switchCase
%type <defaultCase> defaultCase
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
%type <value> lvalue
%type <value> templateValue
%type <valueList> nonEmptyCallArgValueList
%type <valueList> callArgValueList
%type <value> atomicValue
%type <value> unaryValue
%type <value> unaryValueOrNext
%type <value> callValue
%type <value> callValueOrNext
%type <value> multiplyOperatorValue
%type <value> multiplyOperatorValueOrNext
%type <value> addOperatorValue
%type <value> addOperatorValueOrNext
%type <value> bitwiseAndValue
%type <value> bitwiseAndValueOrNext
%type <value> bitwiseOrValue
%type <value> bitwiseOrValueOrNext
%type <value> comparisonOperatorValue
%type <value> comparisonOperatorValueOrNext
%type <value> logicalAndValue
%type <value> logicalAndValueOrNext
%type <value> logicalAndShortCircuitValue
%type <value> logicalAndShortCircuitValueOrNext
%type <value> logicalOrValue
%type <value> logicalOrValueOrNext
%type <value> logicalOrShortCircuitValue
%type <value> logicalOrShortCircuitValueOrNext
%type <value> shiftOperatorValue
%type <value> ternaryOperatorValue
%type <value> ternaryOperatorValueOperand

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
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::NamespaceDecl(parserContext->getCString(""), GETSYM($1))));
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
	| namespaceData alias
	{
		(GETSYM($1))->aliases.push_back(GETSYM($2));
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
	| namespaceData staticAssert
	{
		(GETSYM($1))->staticAsserts.push_back(GETSYM($2));
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
		$$ = $1;
	}
	;

moduleName:
	moduleNameComponent
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::StringList(1, $1)));
	}
	| moduleName DOT moduleNameComponent
	{
		(GETSYM($1))->push_back($3);
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
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::NamespaceDecl($2, GETSYM($4))));
	}
	;

nonTemplatedAlias:
	USING NAME SETEQUAL value SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::AliasDecl($2, GETSYM($4))));
	}
	;

alias:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET requireSpecifier nonTemplatedAlias
	{
		(GETSYM($6))->setTemplateVariables(GETSYM($3));
		(GETSYM($6))->setRequireSpecifier(GETSYM($5));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($6)).get()));
	}
	| nonTemplatedAlias
	{
		$$ = $1;
	}
	;

cTypeVarList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeVarList()));
	}
	| cTypeVarList typeVar SEMICOLON
	{
		(GETSYM($1))->push_back(GETSYM($2));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	| cTypeVarList SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

staticAssert:
	STATIC ASSERT predicateExpr SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::StaticAssert(GETSYM($3))));
	}
	;

constSpecifier:
	/* empty */
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ConstSpecifier::None()));
	}
	| CONST
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ConstSpecifier::Const()));
	}
	| CONST LROUNDBRACKET predicateExpr RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ConstSpecifier::Expr(GETSYM($3))));
	}
	;

predicateExprAtom:
	TRUEVAL
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Predicate::True()));
	}
	| FALSEVAL
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Predicate::False()));
	}
	| LROUNDBRACKET predicateExpr RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Predicate::Bracket(GETSYM($2))));
	}
	| type COLON type
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Predicate::TypeSpec(GETSYM($1), GETSYM($3))));
	}
	| symbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Predicate::Symbol(GETSYM($1))));
	}
	;

predicateExprBinaryOperator:
	predicateExprAtom
	{
		$$ = $1;
	}
	| predicateExprBinaryOperator AND predicateExprAtom
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Predicate::And(GETSYM($1), GETSYM($3))));
	}
	| predicateExprBinaryOperator OR predicateExprAtom
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Predicate::Or(GETSYM($1), GETSYM($3))));
	}
	;

predicateExpr:
	predicateExprBinaryOperator
	{
		$$ = $1;
	}
	;

moveSpecifier:
	/* empty */
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::RequireSpecifier::None()));
	}
	| MOVE LROUNDBRACKET predicateExpr RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::RequireSpecifier::Expr(GETSYM($3))));
	}
	;

noexceptSpecifier:
	/* empty */
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::RequireSpecifier::None()));
	}
	| NOEXCEPT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::RequireSpecifier::NoPredicate()));
	}
	| NOEXCEPT LROUNDBRACKET predicateExpr RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::RequireSpecifier::Expr(GETSYM($3))));
	}
	;

requireSpecifier:
	/* empty */
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::RequireSpecifier::None()));
	}
	| REQUIRE LROUNDBRACKET predicateExpr RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::RequireSpecifier::Expr(GETSYM($3))));
	}
	;

nonTemplatedFunctionDecl:
	type functionName LROUNDBRACKET typeVarList RROUNDBRACKET constSpecifier noexceptSpecifier requireSpecifier SEMICOLON
	{
		const bool isVarArg = false;
		const bool isStatic = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, isStatic, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($6), GETSYM($7), GETSYM($8))));
	}
	| STATIC type functionName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier requireSpecifier SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticDecl(GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($7), GETSYM($8))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET constSpecifier noexceptSpecifier requireSpecifier SEMICOLON
	{
		const bool isVarArg = true;
		const bool isStatic = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, isStatic, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($9), GETSYM($10), GETSYM($11))));
	}
	
	// Error cases
	| type functionName LROUNDBRACKET typeVarList RROUNDBRACKET constSpecifier noexceptSpecifier requireSpecifier error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@6));
		const bool isVarArg = false;
		const bool isStatic = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, isStatic, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($6), GETSYM($7), GETSYM($8))));
	}
	| STATIC type functionName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier requireSpecifier error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@6));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticDecl(GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($7), GETSYM($8))));
	}
	| type functionName LROUNDBRACKET nonEmptyTypeVarList DOT DOT DOT RROUNDBRACKET constSpecifier noexceptSpecifier requireSpecifier error
	{
		parserContext->error("Function declaration must be terminated with a semicolon.", LOC(&@9));
		const bool isVarArg = true;
		const bool isStatic = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, isStatic, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($9), GETSYM($10), GETSYM($11))));
	}
	;

importedFunctionDecl:
	nonTemplatedFunctionDecl
	{
		$$ = $1;
	}
	| IMPORT nonTemplatedFunctionDecl
	{
		(GETSYM($2))->setImport();
		$$ = $2;
	}
	;

functionDecl:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET requireSpecifier importedFunctionDecl
	{
		(GETSYM($6))->setTemplateVariables(GETSYM($3));
		(GETSYM($6))->setRequireSpecifier(GETSYM($5));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($6)).get()));
	}
	| importedFunctionDecl
	{
		$$ = $1;
	}
	;

nonTemplatedFunctionDef:
	type functionName LROUNDBRACKET typeVarList RROUNDBRACKET constSpecifier noexceptSpecifier requireSpecifier scope
	{
		const bool isVarArg = false;
		const bool isStatic = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Def(isVarArg, isStatic, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($9), GETSYM($6), GETSYM($7), GETSYM($8))));
	}
	| STATIC type functionName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier requireSpecifier scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticDef(GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($9), GETSYM($7), GETSYM($8))));
	}
	;

exportedFunctionDef:
	nonTemplatedFunctionDef
	{
		$$ = $1;
	}
	| EXPORT nonTemplatedFunctionDef
	{
		(GETSYM($2))->setExport();
		$$ = $2;
	}
	;

functionDef:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET requireSpecifier exportedFunctionDef
	{
		(GETSYM($6))->setTemplateVariables(GETSYM($3));
		(GETSYM($6))->setRequireSpecifier(GETSYM($5));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($6)).get()));
	}
	| exportedFunctionDef
	{
		$$ = $1;
	}
	;

nonTemplatedStaticMethodDecl:
	STATIC type methodName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier requireSpecifier SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticDecl(GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($7), GETSYM($8))));
	}
	;

nonTemplatedMethodDecl:
	nonTemplatedStaticMethodDecl
	{
		$$ = $1;
	}
	| type methodName LROUNDBRACKET typeVarList RROUNDBRACKET constSpecifier noexceptSpecifier requireSpecifier SEMICOLON
	{
		const bool isVarArg = false;
		const bool isStatic = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Decl(isVarArg, isStatic, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($6), GETSYM($7), GETSYM($8))));
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

nonTemplatedStaticMethodDef:
	STATIC methodName SETEQUAL DEFAULT requireSpecifier SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::DefaultStaticMethodDef(GETSYM($2), GETSYM($5))));
	}
	| STATIC methodName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier requireSpecifier scope
	{
		const auto returnType = locic::AST::makeNode(LOC(&@1), locic::AST::Type::Auto());
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticDef(returnType, GETSYM($2), GETSYM($4), GETSYM($8), GETSYM($6), GETSYM($7))));
	}
	| STATIC type methodName LROUNDBRACKET typeVarList RROUNDBRACKET noexceptSpecifier requireSpecifier scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::StaticDef(GETSYM($2), GETSYM($3), GETSYM($5), GETSYM($9), GETSYM($7), GETSYM($8))));
	}
	;

nonTemplatedMethodDef:
	nonTemplatedStaticMethodDef
	{
		$$ = $1;
	}
	| destructorName scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Destructor(GETSYM($1), GETSYM($2))));
	}
	| methodName SETEQUAL DEFAULT requireSpecifier SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::DefaultMethodDef(GETSYM($1), GETSYM($4))));
	}
	| type methodName LROUNDBRACKET typeVarList RROUNDBRACKET constSpecifier noexceptSpecifier requireSpecifier scope
	{
		const bool isVarArg = false;
		const bool isStatic = false;
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Function::Def(isVarArg, isStatic, GETSYM($1), GETSYM($2), GETSYM($4), GETSYM($9), GETSYM($6), GETSYM($7), GETSYM($8))));
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
	type NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TemplateTypeVar::NoSpec(GETSYM($1), $2)));
	}
	| type NAME COLON type
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TemplateTypeVar::WithSpec(GETSYM($1), $2, GETSYM($4))));
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

noTagSet:
	/* empty */
	{
		$$ = MAKESYM(locic::AST::Node<locic::AST::StringList>());
	}
	| NOTAG LROUNDBRACKET stringList RROUNDBRACKET
	{
		$$ = $3;
	}
	;

typeInstance:
	TEMPLATE LTRIBRACKET templateTypeVarList RTRIBRACKET requireSpecifier moveSpecifier noTagSet nonTemplatedTypeInstance
	{
		(GETSYM($8))->setTemplateVariables(GETSYM($3));
		(GETSYM($8))->setRequireSpecifier(GETSYM($5));
		(GETSYM($8))->setMoveSpecifier(GETSYM($6));
		(GETSYM($8))->setNoTagSet(GETSYM($7));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($8)).get()));
	}
	| nonTemplatedTypeInstance
	{
		$$ = $1;
	}
	;

unionDatatypeEntry:
	NAME LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Datatype($1, GETSYM($3))));
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
	| COLON symbol LROUNDBRACKET callArgValueList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::ExceptionInitializer::Initialize(GETSYM($2), GETSYM($4))));
	}
	;

stringList:
	NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::StringList(1, $1)));
	}
	| stringList COMMA NAME
	{
		(GETSYM($1))->push_back($3);
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

nonTemplatedTypeInstance:
	PRIMITIVE NAME LCURLYBRACKET methodDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Primitive($2, GETSYM($4))));
	}
	| ENUM NAME LCURLYBRACKET stringList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Enum($2, GETSYM($4))));
	}
	| STRUCT NAME SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::OpaqueStruct($2)));
	}
	| STRUCT NAME LCURLYBRACKET cTypeVarList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Struct($2, GETSYM($4))));
	}
	| UNION NAME LCURLYBRACKET cTypeVarList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Union($2, GETSYM($4))));
	}
	| CLASS NAME LCURLYBRACKET methodDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::ClassDecl($2, GETSYM($4))));
	}
	| CLASS NAME LROUNDBRACKET typeVarList RROUNDBRACKET LCURLYBRACKET methodDefList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::ClassDef($2, GETSYM($4), GETSYM($7))));
	}
	| INTERFACE NAME LCURLYBRACKET methodDeclList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Interface($2, GETSYM($4))));
	}
	| DATATYPE NAME LROUNDBRACKET typeVarList RROUNDBRACKET SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Datatype($2, GETSYM($4))));
	}
	| DATATYPE NAME SETEQUAL unionDatatypeEntryList SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::UnionDatatype($2, GETSYM($4))));
	}
	| EXCEPTION NAME LROUNDBRACKET typeVarList RROUNDBRACKET exceptionInitializer SEMICOLON
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeInstance::Exception($2, GETSYM($4), GETSYM($6))));
	}
	;

functionNameElement:
	NAME
	{
		$$ = $1;
	}
	| MOVE
	{
		$$ = parserContext->getCString("move");
	}
	| NULLVAL
	{
		$$ = parserContext->getCString("null");
	}
	;

functionName:
	functionNameElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::Name(locic::Name::Relative() + $1)));
	}
	| functionName DOUBLE_COLON functionNameElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::Name(*(GETSYM($1)) + $3)));
	}
	;

methodName:
	functionNameElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::Name(locic::Name::Relative() + $1)));
	}
	;


destructorNameElement:
	TILDA
	{
		$$ = parserContext->getCString("__destroy");
	}
	;

destructorName:
	destructorNameElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::Name(locic::Name::Relative() + $1)));
	}
	;

symbolElement:
	NAME
	{
		const auto emptyArgList = locic::AST::makeNode(LOC(&@1), new locic::AST::ValueList());
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement($1, emptyArgList)));
	}
	| NAME LTRIBRACKET templateValueList RTRIBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement($1, GETSYM($3))));
	}
	| TYPENAME emptyTemplateArgumentList
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::SymbolElement(parserContext->getCString("typename_t"), GETSYM($2))));
	}
	;

symbol:
	symbolElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Symbol(locic::AST::Symbol::Relative() + GETSYM($1))));
	}
	| symbol DOUBLE_COLON symbolElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::Symbol(*(GETSYM($1)) + GETSYM($3))));
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
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, parserContext->getCString("byte"))));
	}
	| UBYTE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::UNSIGNED, parserContext->getCString("byte"))));
	}
	| optionalSignedModifier SHORT optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, parserContext->getCString("short"))));
	}
	| USHORT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::UNSIGNED, parserContext->getCString("short"))));
	}
	| optionalSignedModifier INT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, parserContext->getCString("int"))));
	}
	| UINT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::UNSIGNED, parserContext->getCString("int"))));
	}
	| SIGNED
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::SIGNED, parserContext->getCString("int"))));
	}
	| UNSIGNED
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::UNSIGNED, parserContext->getCString("int"))));
	}
	
	// Handle ridiculous numbers of shift/reduce conflicts with
	// 'long double' by splitting between cases where signed
	// is and isn't specified (so that 'LONG' always means shift).
	| LONG optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::NO_SIGNED, parserContext->getCString("long"))));
	}
	| signedModifier LONG optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, parserContext->getCString("long"))));
	}
	| ULONG
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::UNSIGNED, parserContext->getCString("long"))));
	}
	| LONG LONG optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::NO_SIGNED, parserContext->getCString("longlong"))));
	}
	| signedModifier LONG LONG optionalInt
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, parserContext->getCString("longlong"))));
	}
	| signedModifier LONGLONG
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer($1, parserContext->getCString("longlong"))));
	}
	| ULONGLONG
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Integer(locic::AST::Type::UNSIGNED, parserContext->getCString("longlong"))));
	}
	;

floatType:
	FLOAT
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Float(parserContext->getCString("float"))));
	}
	| DOUBLE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Float(parserContext->getCString("double"))));
	}
	| LONG DOUBLE
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Float(parserContext->getCString("longdouble"))));
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
	| LROUNDBRACKET STAR RROUNDBRACKET LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET typeList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Function(GETSYM($5), GETSYM($8))));
	}
	| LROUNDBRACKET STAR RROUNDBRACKET LROUNDBRACKET type RROUNDBRACKET LROUNDBRACKET nonEmptyTypeList COMMA DOT DOT DOT RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::VarArgFunction(GETSYM($5), GETSYM($8))));
	}
	// To be brought back if existing reduce/reduce conflicts can be eliminated...
	/*| error
	{
		parserContext->error("Invalid type.", LOC(&@1));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Auto()));
	}*/
	;

typePrecision3:
	typePrecision4
	{
		$$ = $1;
	}
	| CONST typePrecision4WithSymbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Const(GETSYM($2))));
	}
	| CONST LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Const(GETSYM($3))));
	}
	| CONST LTRIBRACKET predicateExpr RTRIBRACKET typePrecision4WithSymbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::ConstPredicate(GETSYM($3), GETSYM($5))));
	}
	| CONST LTRIBRACKET predicateExpr RTRIBRACKET LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::ConstPredicate(GETSYM($3), GETSYM($6))));
	}
	| NOTAG typePrecision4WithSymbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::NoTag(GETSYM($2))));
	}
	| NOTAG LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::NoTag(GETSYM($3))));
	}
	;

typePrecision2:
	typePrecision3
	{
		$$ = $1;
	}
	| LVAL LTRIBRACKET type RTRIBRACKET typePrecision3WithSymbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Lval(GETSYM($3), GETSYM($5))));
	}
	| LVAL LTRIBRACKET type RTRIBRACKET LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Lval(GETSYM($3), GETSYM($6))));
	}
	| REF LTRIBRACKET type RTRIBRACKET typePrecision3WithSymbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Ref(GETSYM($3), GETSYM($5))));
	}
	| REF LTRIBRACKET type RTRIBRACKET LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Ref(GETSYM($3), GETSYM($6))));
	}
	| STATICREF LTRIBRACKET type RTRIBRACKET typePrecision3WithSymbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::StaticRef(GETSYM($3), GETSYM($5))));
	}
	| STATICREF LTRIBRACKET type RTRIBRACKET LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::StaticRef(GETSYM($3), GETSYM($6))));
	}
	;

pointerType:
	typePrecision1WithSymbol STAR
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Pointer(GETSYM($1))));
	}
	| typePrecision1WithSymbol AMPERSAND
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::Reference(GETSYM($1))));
	}
	| typePrecision1WithSymbol LSQUAREBRACKET templateValue RSQUAREBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Type::StaticArray(GETSYM($1), GETSYM($3))));
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
	;

typePrecision0:
	typePrecision1
	{
		$$ = $1;
	}
	;

typeValue:
	typePrecision0
	{
		$$ = $1;
	}
	;

typePrecision4WithSymbol:
	typePrecision4
	{
		$$ = $1;
	}
	| objectType
	{
		$$ = $1;
	}
	;

typePrecision3WithSymbol:
	typePrecision3
	{
		$$ = $1;
	}
	| objectType
	{
		$$ = $1;
	}
	;

typePrecision1WithSymbol:
	typePrecision1
	{
		$$ = $1;
	}
	| objectType
	{
		$$ = $1;
	}
	;

typePrecision0WithSymbol:
	typePrecision0
	{
		$$ = $1;
	}
	| objectType
	{
		$$ = $1;
	}
	;

type:
	typePrecision0WithSymbol
	{
		$$ = $1;
	}
	;

emptyTypeList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::TypeList()));
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
	emptyTypeList
	{
		$$ = $1;
	}
	| nonEmptyTypeList
	{
		$$ = $1;
	}
	;

emptyTemplateArgumentList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::ValueList()));
	}
	;

templateArgumentList:
	templateValue
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::ValueList(1, GETSYM($1))));
	}
	| templateArgumentList COMMA templateValue
	{
		(GETSYM($1))->push_back(GETSYM($3));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($1)).get()));
	}
	;

// Template value lists are actually considered as tuples, and brackets can be
// added to resolve ambiguities; from a syntactic point of view no-brackets is
// actually the special case.
templateValueList:
	templateArgumentList
	{
		$$ = $1;
	}
	| LROUNDBRACKET templateArgumentList COMMA RROUNDBRACKET
	{
		$$ = $2;
	}
	| LROUNDBRACKET templateArgumentList COMMA templateValue RROUNDBRACKET
	{
		(GETSYM($2))->push_back(GETSYM($4));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), (GETSYM($2)).get()));
	}
	;

patternTypeVar:
	constModifiedObjectType LROUNDBRACKET typeVarList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeVar::PatternVar(GETSYM($1), GETSYM($3))));
	}
	;
	
basicTypeVar:
	type NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::TypeVar::NamedVar(GETSYM($1), $2)));
	}
	;

overrideConstTypeVar:
	basicTypeVar
	{
		$$ = $1;
	}
	| OVERRIDE_CONST basicTypeVar
	{
		(GETSYM($2))->setOverrideConst();
		$$ = $2;
	}
	;

finalTypeVar:
	overrideConstTypeVar
	{
		$$ = $1;
	}
	| FINAL overrideConstTypeVar
	{
		(GETSYM($2))->setFinal();
		$$ = $2;
	}
	;

unusedTypeVar:
	finalTypeVar
	{
		$$ = $1;
	}
	| UNUSED finalTypeVar
	{
		(GETSYM($2))->setUnused();
		$$ = $2;
	}
	;

typeVar:
	unusedTypeVar
	{
		$$ = $1;
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
	
callArgValueList:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::ValueList()));
	}
	| nonEmptyCallArgValueList
	{
		$$ = $1;
	}
	;
	
/* There are some cases where it's possible to have an ambiguity due to a comma
 * in a template argument list being treated as a comma separating the
 * arguments. For example:
 * 
 *     f(A<B, C>(D))
 * 
 * In this case in could be treated as one of:
 * 
 *     f( (A<B) , (C>(D)) ) : Passing two arguments.
 * 
 *     or
 * 
 *     f( (A<B, C>(D)) ) : Passing one argument.
 * 
 * In this case we favour the latter, assuming the user meant to construct an
 * object as an argument. This means we merge any ambiguities by always
 * favouring the smaller argument list.
 */
nonEmptyCallArgValueList:
	value %dprec 2
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::ValueList(1, GETSYM($1))));
	}
	| nonEmptyCallArgValueList COMMA value %dprec 1 %merge <mergeCallArgList>
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::AST::ValueList()));
		for (const auto& node: *(GETSYM($1))) {
			(GETSYM($$))->push_back(node);
		}
		(GETSYM($$))->push_back(GETSYM($3));
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

defaultCase:
	// empty
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::DefaultCase::Empty()));
	}
	| DEFAULT scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::DefaultCase::Scope(GETSYM($2))));
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
	| SWITCH LROUNDBRACKET value RROUNDBRACKET LCURLYBRACKET switchCaseList defaultCase RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Switch(GETSYM($3), GETSYM($6), GETSYM($7))));
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
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ScopeExit($3, GETSYM($5))));
	}
	| ASSERT NOEXCEPT scope
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::AssertNoExcept(GETSYM($3))));
	}
	;
	
normalStatement:
	unusedTypeVar SETEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::VarDecl(GETSYM($1), GETSYM($3))));
	}
	| LET patternTypeVar SETEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::VarDecl(GETSYM($2), GETSYM($4))));
	}
	| lvalue SETEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Assign(locic::AST::ASSIGN_DIRECT, GETSYM($1), GETSYM($3))));
	}
	| lvalue ADDEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Assign(locic::AST::ASSIGN_ADD, GETSYM($1), GETSYM($3))));
	}
	| lvalue SUBEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Assign(locic::AST::ASSIGN_SUB, GETSYM($1), GETSYM($3))));
	}
	| lvalue MULEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Assign(locic::AST::ASSIGN_MUL, GETSYM($1), GETSYM($3))));
	}
	| lvalue DIVEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Assign(locic::AST::ASSIGN_DIV, GETSYM($1), GETSYM($3))));
	}
	| lvalue PERCENTEQUAL value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Assign(locic::AST::ASSIGN_MOD, GETSYM($1), GETSYM($3))));
	}
	| lvalue DOUBLE_PLUS
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Increment(GETSYM($1))));
	}
	| lvalue DOUBLE_MINUS
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::Decrement(GETSYM($1))));
	}
	| unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmt(GETSYM($1))));
	}
	| UNUSED_RESULT value
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Statement::ValueStmtVoidCast(GETSYM($2))));
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
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::Constant($1)));
	}
	| TRUEVAL
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::Constant(locic::Constant::True())));
	}
	| FALSEVAL
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::Constant(locic::Constant::False())));
	}
	| NULLVAL
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), new locic::Constant(locic::Constant::Null())));
	}
	;

castKind:
	CONST_CAST
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
	
atomicValue:
	LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Bracket(GETSYM($2))));
	}
	| symbol
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::SymbolRef(GETSYM($1))));
	}
	| AT NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::MemberRef($2)));
	}
	| AT emptyTemplateArgumentList LROUNDBRACKET callArgValueList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::InternalConstruct(GETSYM($2), GETSYM($4))));
	}
	| AT LTRIBRACKET templateValueList RTRIBRACKET LROUNDBRACKET callArgValueList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::InternalConstruct(GETSYM($3), GETSYM($6))));
	}
	| constant
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Literal(parserContext->getCString(""), GETSYM($1))));
	}
	| NAME constant
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Literal($1, GETSYM($2))));
	}
	| constant NAME
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Literal($2, GETSYM($1))));
	}
	| castKind LTRIBRACKET type COMMA type RTRIBRACKET LROUNDBRACKET value RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Cast($1, GETSYM($3), GETSYM($5), GETSYM($8))));
	}
	
	// TODO: Remove the following...
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
	| ALIGNOF LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::AlignOf(GETSYM($3))));
	}
	| SIZEOF LROUNDBRACKET type RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::SizeOf(GETSYM($3))));
	}
	;
	
callValue:
	callValueOrNext DOT functionNameElement
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::MemberAccess(GETSYM($1), $3)));
	}
	| callValueOrNext DOT functionNameElement LTRIBRACKET templateValueList RTRIBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::TemplatedMemberAccess(GETSYM($1), $3, GETSYM($5))));
	}
	| callValueOrNext PTRACCESS functionNameElement
	{
		const auto derefNode = locic::AST::makeNode(LOC(&@$), UnaryOp(parserContext->getCString("deref"), GETSYM($1)));
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::MemberAccess(derefNode, $3)));
	}
	| callValueOrNext LROUNDBRACKET callArgValueList RROUNDBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::FunctionCall(GETSYM($1), GETSYM($3))));
	}
	| callValueOrNext LSQUAREBRACKET value RSQUAREBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp(parserContext->getCString("index"), GETSYM($1), GETSYM($3))));
	}
	;
	
callValueOrNext:
	callValue
	{
		$$ = $1;
	}
	| atomicValue
	{
		$$ = $1;
	}
	;
	
unaryValue:
	PLUS unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::UnaryOp(locic::AST::OP_PLUS, GETSYM($2))));
	}
	| MINUS unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::UnaryOp(locic::AST::OP_MINUS, GETSYM($2))));
	}
	| EXCLAIMMARK unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::UnaryOp(locic::AST::OP_NOT, GETSYM($2))));
	}
	| AMPERSAND unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::UnaryOp(locic::AST::OP_ADDRESS, GETSYM($2))));
	}
	| STAR unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::UnaryOp(locic::AST::OP_DEREF, GETSYM($2))));
	}
	| MOVE unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::UnaryOp(locic::AST::OP_MOVE, GETSYM($2))));
	}
	;
	
unaryValueOrNext:
	unaryValue
	{
		$$ = $1;
	}
	| callValueOrNext
	{
		$$ = $1;
	}
	;
	
multiplyOperatorValue:
	multiplyOperatorValueOrNext STAR unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp(parserContext->getCString("multiply"), GETSYM($1), GETSYM($3))));
	}
	| multiplyOperatorValueOrNext FORWARDSLASH unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp(parserContext->getCString("divide"), GETSYM($1), GETSYM($3))));
	}
	| multiplyOperatorValueOrNext PERCENT unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), BinaryOp(parserContext->getCString("modulo"), GETSYM($1), GETSYM($3))));
	}
	;
	
multiplyOperatorValueOrNext:
	multiplyOperatorValue
	{
		$$ = $1;
	}
	| unaryValueOrNext
	{
		$$ = $1;
	}
	;
	
addOperatorValue:
	addOperatorValueOrNext PLUS multiplyOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_ADD, GETSYM($1), GETSYM($3))));
	}
	| addOperatorValueOrNext MINUS multiplyOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_SUBTRACT, GETSYM($1), GETSYM($3))));
	}
	;
	
addOperatorValueOrNext:
	addOperatorValue
	{
		$$ = $1;
	}
	| multiplyOperatorValueOrNext
	{
		$$ = $1;
	}
	;
	
shiftOperatorValue:
	unaryValueOrNext DOUBLE_LTRIBRACKET unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LEFTSHIFT, GETSYM($1), GETSYM($3))));
	}
	| unaryValueOrNext RTRIBRACKET RTRIBRACKET unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_RIGHTSHIFT, GETSYM($1), GETSYM($4))));
	}
	;
	
comparisonOperatorValue:
	addOperatorValueOrNext ISEQUAL addOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_ISEQUAL, GETSYM($1), GETSYM($3))));
	}
	| addOperatorValueOrNext NOTEQUAL addOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_NOTEQUAL, GETSYM($1), GETSYM($3))));
	}
	| addOperatorValueOrNext LTRIBRACKET addOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LESSTHAN, GETSYM($1), GETSYM($3))));
	}
	| addOperatorValueOrNext RTRIBRACKET addOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_GREATERTHAN, GETSYM($1), GETSYM($3))));
	}
	| addOperatorValueOrNext LESSOREQUAL addOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LESSTHANOREQUAL, GETSYM($1), GETSYM($3))));
	}
	| addOperatorValueOrNext GREATEROREQUAL addOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_GREATERTHANOREQUAL, GETSYM($1), GETSYM($3))));
	}
	| type COLON type
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::CapabilityTest(GETSYM($1), GETSYM($3))));
	}
	;
	
comparisonOperatorValueOrNext:
	comparisonOperatorValue
	{
		$$ = $1;
	}
	// Use a unary value as the next level down to avoid conditions like 'a + b'
	// or 'a * b'.
	| unaryValueOrNext
	{
		$$ = $1;
	}
	;
	
bitwiseAndValue:
	bitwiseAndValueOrNext AMPERSAND unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_BITWISEAND, GETSYM($1), GETSYM($3))));
	}
	;
	
bitwiseAndValueOrNext:
	bitwiseAndValue
	{
		$$ = $1;
	}
	| unaryValueOrNext
	{
		$$ = $1;
	}
	;
	
bitwiseOrValue:
	bitwiseOrValueOrNext VERTICAL_BAR unaryValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_BITWISEOR, GETSYM($1), GETSYM($3))));
	}
	;
	
bitwiseOrValueOrNext:
	bitwiseOrValue
	{
		$$ = $1;
	}
	| unaryValueOrNext
	{
		$$ = $1;
	}
	;
	
logicalAndShortCircuitValue:
	logicalAndShortCircuitValueOrNext DOUBLE_AMPERSAND comparisonOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LOGICALAND, GETSYM($1), GETSYM($3))));
	}
	;

logicalAndShortCircuitValueOrNext:
	logicalAndShortCircuitValue
	{
		$$ = $1;
	}
	| comparisonOperatorValueOrNext
	{
		$$ = $1;
	}
	;

logicalAndValue:
	logicalAndValueOrNext AND comparisonOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LOGICALAND, GETSYM($1), GETSYM($3))));
	}
	;
	
logicalAndValueOrNext:
	logicalAndValue
	{
		$$ = $1;
	}
	| comparisonOperatorValueOrNext
	{
		$$ = $1;
	}
	;

logicalOrShortCircuitValue:
	logicalOrShortCircuitValueOrNext DOUBLE_VERTICAL_BAR comparisonOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LOGICALOR, GETSYM($1), GETSYM($3))));
	}
	;

logicalOrShortCircuitValueOrNext:
	logicalOrShortCircuitValue
	{
		$$ = $1;
	}
	| comparisonOperatorValueOrNext
	{
		$$ = $1;
	}
	;

logicalOrValue:
	logicalOrValueOrNext OR comparisonOperatorValueOrNext
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::BinaryOp(locic::AST::OP_LOGICALOR, GETSYM($1), GETSYM($3))));
	}
	;
	
logicalOrValueOrNext:
	logicalOrValue
	{
		$$ = $1;
	}
	| comparisonOperatorValueOrNext
	{
		$$ = $1;
	}
	;
	
ternaryOperatorValueOperand:
	atomicValue
	{
		$$ = $1;
	}
	| callValue
	{
		$$ = $1;
	}
	| unaryValue
	{
		$$ = $1;
	}
	| multiplyOperatorValue
	{
		$$ = $1;
	}
	| addOperatorValue
	{
		$$ = $1;
	}
	| shiftOperatorValue
	{
		$$ = $1;
	}
	| bitwiseAndValue
	{
		$$ = $1;
	}
	| bitwiseOrValue
	{
		$$ = $1;
	}
	;
	
ternaryOperatorValue:
	comparisonOperatorValueOrNext QUESTIONMARK ternaryOperatorValueOperand COLON ternaryOperatorValueOperand
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::Ternary(GETSYM($1), GETSYM($3), GETSYM($5))));
	}
	;
	
value:
	atomicValue
	{
		$$ = $1;
	}
	| callValue %merge <mergeValue>
	{
		$$ = $1;
	}
	| unaryValue
	{
		$$ = $1;
	}
	| multiplyOperatorValue
	{
		$$ = $1;
	}
	| addOperatorValue
	{
		$$ = $1;
	}
	| comparisonOperatorValue
	{
		$$ = $1;
	}
	| shiftOperatorValue
	{
		$$ = $1;
	}
	| bitwiseAndValue
	{
		$$ = $1;
	}
	| bitwiseOrValue
	{
		$$ = $1;
	}
	| logicalAndValue
	{
		$$ = $1;
	}
	| logicalAndShortCircuitValue
	{
		$$ = $1;
	}
	| logicalOrValue
	{
		$$ = $1;
	}
	| logicalOrShortCircuitValue
	{
		$$ = $1;
	}
	| ternaryOperatorValue
	{
		$$ = $1;
	}
	| typeValue %merge <mergeValue>
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::TypeRef(GETSYM($1))));
	}
	| LCURLYBRACKET callArgValueList RCURLYBRACKET
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::ArrayLiteral(GETSYM($2))));
	}
	;
	
lvalue:
	atomicValue
	{
		$$ = $1;
	}
	| callValue
	{
		$$ = $1;
	}
	| unaryValue
	{
		$$ = $1;
	}
	;
	
templateValue:
	atomicValue
	{
		$$ = $1;
	}
	| unaryValue
	{
		$$ = $1;
	}
	| multiplyOperatorValue
	{
		$$ = $1;
	}
	| addOperatorValue
	{
		$$ = $1;
	}
	| bitwiseAndValue
	{
		$$ = $1;
	}
	| bitwiseOrValue
	{
		$$ = $1;
	}
	| typeValue
	{
		$$ = MAKESYM(locic::AST::makeNode(LOC(&@$), locic::AST::Value::TypeRef(GETSYM($1))));
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

