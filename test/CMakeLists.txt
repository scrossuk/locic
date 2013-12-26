# Compiler tests.
# Run 'make test' or 'ctest' to execute these.

macro(test_check_output name)
	add_test(NAME TestCheckOutput${name}
		COMMAND ${CMAKE_BINARY_DIR}/tools/loci-test ${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci --expected-output=${CMAKE_CURRENT_SOURCE_DIR}/${name}.log)
endmacro(test_check_output)

add_subdirectory(Class)
add_subdirectory(Interface)

