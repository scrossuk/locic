
# Need to include some generated files.
include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

# The flex-generated lexer contains the 'register' keyword, which triggers various errors; hide these.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register")

# Generate the parser.
add_custom_command(OUTPUT GeneratedParser.cpp GeneratedParser.hpp ParserReport.txt
	COMMAND bison --report=all --report-file=ParserReport.txt --defines=GeneratedParser.hpp --output=GeneratedParser.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate Lexer (depends on parser generation for token values).
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/GeneratedLexer.cpp
	COMMAND flex -o GeneratedLexer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/GeneratedParser.hpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(locic-parser
	DefaultParser.cpp
	${CMAKE_CURRENT_BINARY_DIR}/GeneratedLexer.cpp
	${CMAKE_CURRENT_BINARY_DIR}/GeneratedParser.cpp
)

