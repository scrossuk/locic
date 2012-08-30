# Copy Parser.y from source directory to build directory
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Parser.y COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y ${CMAKE_CURRENT_BINARY_DIR}/Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y)

set_source_files_properties(Parser.c PROPERTIES LANGUAGE CXX )

# Generate Parser
add_custom_command(OUTPUT Parser.c Parser.h Parser.out COMMAND lemon Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/Parser.y)

# Generate Lexer (depends on Parser.h for symbols)
add_custom_command(OUTPUT Lexer.cpp COMMAND flex -o Lexer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l MAIN_DEPENDENCY Lexer.l DEPENDS Parser.h)

# Find LLVM (for code generation)
find_package(LLVM REQUIRED)

add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

SET(CMAKE_BUILD_TYPE Debug)

# Build semantic analysis.
add_subdirectory(SemanticAnalysis)

# Build code generator.
add_subdirectory(CodeGen)

# Compile
add_executable(locic main.cpp Lexer.cpp Parser.c)

# Link with LLVM
link_directories(${LLVM_LIBRARY_DIRS})
llvm_map_components_to_libraries(REQ_LLVM_LIBRARIES core native)
target_link_libraries(locic locic-semanalysis locic-codegen clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST clangLex clangBasic clangDriver clangSerialization ${REQ_LLVM_LIBRARIES})

