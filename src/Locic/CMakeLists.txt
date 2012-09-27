# Find LLVM (for code generation)
find_package(LLVM REQUIRED)

add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

SET(CMAKE_BUILD_TYPE Debug)

# Build parser.
add_subdirectory(Parser)

# Build semantic tree structures.
add_subdirectory(SEM)

# Build semantic analysis.
add_subdirectory(SemanticAnalysis)

# Build code generator.
add_subdirectory(CodeGen)

# Compile
add_executable(locic main.cpp)

# Link with LLVM
link_directories(${LLVM_LIBRARY_DIRS})
llvm_map_components_to_libraries(REQ_LLVM_LIBRARIES core native)
target_link_libraries(locic locic-parser locic-semanalysis locic-codegen locic-sem clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST clangLex clangBasic clangDriver clangSerialization ${REQ_LLVM_LIBRARIES})

