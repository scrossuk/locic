set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCIC_BUILD_WARNINGS}")

# Build abstract syntax tree structures.
add_subdirectory(AST)

# Build code generator.
add_subdirectory(CodeGen)

# Build frontend.
add_subdirectory(Frontend)

# Build lexer.
add_subdirectory(Lex)

# Build parser.
add_subdirectory(Parser)

# Build semantic analysis.
add_subdirectory(SemanticAnalysis)

# Build support library.
add_subdirectory(Support)

