cmake_minimum_required(VERSION 2.8)

# Find LLVM (for code generation).
find_package(LLVM REQUIRED)

link_directories(${LLVM_LIBRARY_DIRS})
llvm_map_components_to_libraries(LLVM_REQ_LIBS bitreader bitwriter core native ipo)

# Clang.
set(CLANG_REQ_LIBS clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST clangLex clangBasic clangDriver clangSerialization)

# Compiler flags.
add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings -Werror")

# Use C++11.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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

set(REQ_LIBS
	${CLANG_REQ_LIBS}
	${LLVM_REQ_LIBS}
	boost_filesystem
	boost_program_options
	boost_system
	)

# Use 'xxd' to convert 'BuiltInTypes.loci'
# into a C string constant that can be
# built into the compiler.
add_custom_command(OUTPUT BuiltInTypes.loci.c
	COMMAND xxd -i BuiltInTypes.loci ${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci.c
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/BuiltInTypes.loci
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(BuiltInTypes DEPENDS BuiltInTypes.loci.c)

# Build vtable generator.
add_executable(calculateVTable
	calculateVTable.cpp
	Log.cpp
	String.cpp
	)

target_link_libraries(calculateVTable
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	${REQ_LIBS}
	)

# Build compiler.
add_executable(locic
	BuiltInTypes.loci.c
	locic.cpp
	Log.cpp
	String.cpp
	)

target_link_libraries(locic
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-ast
	${REQ_LIBS}
	)

# Build test tool.
add_executable(loci-test
	BuiltInTypes.loci.c
	loci_test.cpp
	Log.cpp
	String.cpp
	)

target_link_libraries(loci-test
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-ast
	${REQ_LIBS}
	)

