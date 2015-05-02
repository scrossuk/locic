# Compiler tests for programs that are expected to
# successfully compile.

macro(test_check_success name)
	add_test(NAME TestCheckSuccess${name}
		COMMAND ${CMAKE_BINARY_DIR}/tools/loci-test --test-name TestCheckOutput${name} ${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci)
endmacro(test_check_success)

add_subdirectory(Cast)
add_subdirectory(Const)
add_subdirectory(Exception)
add_subdirectory(Templates)
add_subdirectory(Unused)

