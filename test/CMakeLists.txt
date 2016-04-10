# Compiler tests.
# Run 'make test' or 'ctest' to execute these.

add_test(NAME "RunLitTests"
	COMMAND lit -v -j1 ${CMAKE_CURRENT_SOURCE_DIR}/LitTests)
