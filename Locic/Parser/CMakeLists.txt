cmake_minimum_required(VERSION 2.8)

# Copy Parser.y from source directory to build directory.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Parser.y COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y ${CMAKE_CURRENT_BINARY_DIR}/Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y)

# Generate the parser.
add_custom_command(OUTPUT GeneratedParser.cpp GeneratedParser.hpp ParserReport.txt COMMAND bison --report=all --report-file=ParserReport.txt --defines=GeneratedParser.hpp --output=GeneratedParser.cpp Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/Parser.y)

# Generate Lexer (depends on parser generation for token values).
add_custom_command(OUTPUT Lexer.cpp COMMAND flex -o Lexer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l MAIN_DEPENDENCY Lexer.l DEPENDS GeneratedParser.hpp)

add_library(locic-parser DefaultParser.cpp GeneratedParser.cpp Lexer.cpp)

