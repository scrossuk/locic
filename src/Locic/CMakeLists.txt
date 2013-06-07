# Find LLVM (for code generation)
find_package(LLVM REQUIRED)

add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -Wall -Wshadow -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings")

SET(CMAKE_BUILD_TYPE Debug)

# Build parser.
add_subdirectory(Parser)

# Build abstract syntax tree structures.
add_subdirectory(AST)

# Build semantic tree structures.
add_subdirectory(SEM)

# Build semantic analysis.
add_subdirectory(SemanticAnalysis)

# Build code generator.
add_subdirectory(CodeGen)

# Build vtable generator.
add_executable(calculateVTable calculateVTable.cpp String.cpp)
target_link_libraries(calculateVTable locic-parser locic-semanalysis locic-codegen locic-sem clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST clangLex clangBasic clangDriver clangSerialization ${REQ_LLVM_LIBRARIES})

# Build compiler.
add_executable(locic locic.cpp Log.cpp String.cpp)

# Link with LLVM
link_directories(${LLVM_LIBRARY_DIRS})
llvm_map_components_to_libraries(REQ_LLVM_LIBRARIES bitwriter core native ipo)
target_link_libraries(locic locic-parser locic-semanalysis locic-codegen locic-sem locic-ast clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST clangLex clangBasic clangDriver clangSerialization ${REQ_LLVM_LIBRARIES})

macro(copy_loci_source name)
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.loci"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci" "${CMAKE_CURRENT_BINARY_DIR}/${name}.loci"
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci")
endmacro(copy_loci_source)

copy_loci_source(BuiltInTypes)
copy_loci_source(Example1)
copy_loci_source(Example2)

add_custom_command(OUTPUT exampleProgram
		COMMAND ./locic Example1.loci Example2.loci && clang -o exampleProgram output.bc -lm
		DEPENDS calculateVTable locic ${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci ${CMAKE_CURRENT_BINARY_DIR}/Example1.loci ${CMAKE_CURRENT_BINARY_DIR}/Example2.loci)
		
add_custom_target(LocicExampleProgram ALL DEPENDS exampleProgram)

