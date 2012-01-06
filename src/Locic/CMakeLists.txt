# Copy Parser.y from source directory to build directory
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Parser.y COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y ${CMAKE_CURRENT_BINARY_DIR}/Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y)

# Generate Parser
add_custom_command(OUTPUT Parser.c Parser.h Parser.out COMMAND lemon Parser.y MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/Parser.y)

# Generate Lexer (depends on Parser.h for symbols)
add_custom_command(OUTPUT Lexer.c COMMAND flex -o Lexer.c ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l MAIN_DEPENDENCY Lexer.l DEPENDS Parser.h)

# Find LLVM (for code generation)
find_package(LLVM REQUIRED)

add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )
include_directories(${LLVM_INCLUDE_DIRS})

SET(CMAKE_BUILD_TYPE Debug)

# Compile
add_executable(locic main.c Array.cpp List.c SemanticAnalysis.c SemanticContext.c Stack.cpp StringMap.cpp Lexer.c Parser.c CodeGen.cpp AST/ClassDecl.c AST/ClassDef.c AST/Context.c AST/Function.c AST/Module.c AST/Scope.c AST/Statement.c AST/Type.c AST/TypeVar.c AST/Value.c AST/Var.c SEM/ClassDecl.c SEM/ClassDef.c SEM/Function.c SEM/Module.c SEM/ModuleGroup.c SEM/Scope.c SEM/Statement.c SEM/Type.c SEM/Value.c SEM/Var.c)

# Link with LLVM
link_directories(${LLVM_LIBRARY_DIRS})
llvm_map_components_to_libraries(REQ_LLVM_LIBRARIES core native)
target_link_libraries(locic ${REQ_LLVM_LIBRARIES})

