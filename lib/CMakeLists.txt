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
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci.c
	COMMAND xxd -i BuiltInTypes.loci ${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci.c
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/BuiltInTypes.loci
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

add_dependencies(locic BuiltInTypes)

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

add_dependencies(loci-test BuiltInTypes)

target_link_libraries(loci-test
	locic-parser
	locic-semanalysis
	locic-codegen
	locic-sem
	locic-ast
	${REQ_LIBS}
	)

