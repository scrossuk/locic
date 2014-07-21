# Module tests.

macro(test_multiple_module file1 file2)
	add_test(NAME TestCheckOutput${name}
		COMMAND ${CMAKE_BINARY_DIR}/tools/loci-test --test-name TestCheckOutput${name} --expected-output=${CMAKE_CURRENT_SOURCE_DIR}/${name}.log ${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci)
endmacro(test_multiple_module)

test_check_output(NestedScopeExit)
test_check_output(ScopeException)
test_check_output(ScopeExit)
test_check_output(ScopeSuccessThrow)

