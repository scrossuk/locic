set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOCIC_BUILD_WARNINGS}")

# Build abstract syntax tree structures.
add_subdirectory(AST)

# Build code generator.
add_subdirectory(CodeGen)

# Build parser.
add_subdirectory(Parser)

# Build semantic tree structures.
add_subdirectory(SEM)

# Build semantic analysis.
add_subdirectory(SemanticAnalysis)

# Build support library.
add_subdirectory(Support)

