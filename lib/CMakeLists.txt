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

# Use 'xxd' to convert 'BuiltInTypes.loci'
# into a C string constant that can be
# built into the compiler.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci.c
	COMMAND xxd -i BuiltInTypes.loci ${CMAKE_CURRENT_BINARY_DIR}/BuiltInTypes.loci.c
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/BuiltInTypes.loci
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Build support library.
add_library(locic-support
	BuiltInTypes.loci.c
	Log.cpp
	String.cpp
	)

