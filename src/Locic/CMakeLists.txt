# Copy Parser.y from source directory to build directory
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Parser.y COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y ${CMAKE_CURRENT_BINARY_DIR}/Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y)

# Generate Parser
add_custom_command(OUTPUT Parser.c Parser.h Parser.out COMMAND lemon Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/Parser.y)

# Generate Lexer (depends on Parser.h for symbols)
add_custom_command(OUTPUT Lexer.c COMMAND flex -o Lexer.c ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l MAIN_DEPENDENCY Lexer.l DEPENDS Parser.h)

# Compile
add_executable(locic main.c Lexer.c Parser.c AST/ClassDecl.c AST/ClassDef.c AST/Function.c AST/File.c AST/List.c AST/Scope.c AST/Statement.c AST/Type.c AST/TypeVar.c AST/Value.c AST/Var.c)
