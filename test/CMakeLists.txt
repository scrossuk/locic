# Compiler tests.
# Run 'make test' or 'ctest' to execute these.

macro(test_check_output name)
	add_test(NAME TestCheckOutput${name}
		COMMAND ${CMAKE_BINARY_DIR}/tools/loci-test --test-name TestCheckOutput${name} --expected-output=${CMAKE_CURRENT_SOURCE_DIR}/${name}.log ${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci)
endmacro(test_check_output)

add_subdirectory(Auto)
add_subdirectory(Class)
add_subdirectory(Datatype)
add_subdirectory(Interface)
add_subdirectory(Loop)
add_subdirectory(Lval)

