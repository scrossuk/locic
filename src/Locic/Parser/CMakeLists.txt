# Copy Parser.y from source directory to build directory.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Parser.y COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y ${CMAKE_CURRENT_BINARY_DIR}/Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y)

# Generate the parser.
add_custom_command(OUTPUT Parser.c Parser.h Parser.out COMMAND lemon Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/Parser.y)

# Ensure the parser is compiled as a C++ source file.
set_source_files_properties(Parser.c PROPERTIES LANGUAGE CXX )

# Copy generated file 'Parser.c' to more appropriate 'GeneratedParser.cpp'.
add_custom_command(OUTPUT GeneratedParser.cpp COMMAND ${CMAKE_COMMAND} -E copy_if_different Parser.c GeneratedParser.cpp MAIN_DEPENDENCY Parser.c)

# Copy generated file 'Parser.h' to more appropriate 'TokenValues.hpp'
add_custom_command(OUTPUT TokenValues.hpp COMMAND ${CMAKE_COMMAND} -E copy_if_different Parser.h TokenValues.hpp MAIN_DEPENDENCY Parser.h)

# Generate Lexer (depends on parser generation for token values).
add_custom_command(OUTPUT Lexer.cpp COMMAND flex -o Lexer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l MAIN_DEPENDENCY Lexer.l DEPENDS TokenValues.hpp)

add_library(locic-parser DefaultParser.cpp GeneratedParser.cpp Lexer.cpp)

