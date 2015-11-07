# Compiler tests.
# Run 'make test' or 'ctest' to execute these.

add_subdirectory(CheckOutput)
add_subdirectory(CheckError)
add_subdirectory(CheckSuccess)
add_subdirectory(StdLib)

add_test(NAME "RunLitTests"
	COMMAND lit -v ${CMAKE_CURRENT_SOURCE_DIR}/LitTests)
