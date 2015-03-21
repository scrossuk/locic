# Compiler tests for programs that are expected to
# NOT compile and produce a specific error.

macro(test_check_error name)
	add_test(NAME TestCheckError${name}
		COMMAND ${CMAKE_BINARY_DIR}/tools/loci-test --test-name TestCheckError${name} --expected-error=${CMAKE_CURRENT_SOURCE_DIR}/${name}.log ${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci)
endmacro(test_check_error)

add_subdirectory(Class)
add_subdirectory(Conditional)
add_subdirectory(Const)
add_subdirectory(Datatype)
add_subdirectory(Exception)
add_subdirectory(Function)
add_subdirectory(Loop)
add_subdirectory(NameRef)
add_subdirectory(StaticRef)
add_subdirectory(Templates)
add_subdirectory(Unused)

