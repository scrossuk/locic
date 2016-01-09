
# Need to include some generated files.
include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

# Generate the parser.
add_custom_command(OUTPUT GeneratedParser.cpp GeneratedParser.hpp ParserReport.txt
	COMMAND bison --report=all --report-file=ParserReport.txt --defines=GeneratedParser.hpp --output=GeneratedParser.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(locic-oldparser
	DefaultParser.cpp
	${CMAKE_CURRENT_BINARY_DIR}/GeneratedParser.cpp
)

add_library(locic-parser
	AttributeBuilder.cpp
	AttributeParser.cpp
	DefaultParser.cpp
	FunctionBuilder.cpp
	FunctionParser.cpp
	NamespaceBuilder.cpp
	NamespaceParser.cpp
	PredicateBuilder.cpp
	PredicateParser.cpp
	ScopeParser.cpp
	StatementBuilder.cpp
	StatementParser.cpp
	SymbolBuilder.cpp
	SymbolParser.cpp
	TemplateBuilder.cpp
	TemplateInfo.cpp
	TemplateParser.cpp
	TokenReader.cpp
	TypeBuilder.cpp
	TypeParser.cpp
	TypeInstanceBuilder.cpp
	TypeInstanceParser.cpp
	ValueBuilder.cpp
	ValueParser.cpp
	VarBuilder.cpp
	VarParser.cpp
)

