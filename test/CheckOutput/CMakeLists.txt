# Compiler tests for programs that are expected to
# successfully compile and produce specific output.

macro(test_check_output name)
	cmake_parse_arguments(TEST_CHECK_OUTPUT "EXPECT_FAIL" "" "" ${ARGN})
	set(test_name "TestCheckOutput${name}")
	add_test(NAME "${test_name}"
		COMMAND ${CMAKE_BINARY_DIR}/tools/loci-test --test-name "${test_name}" --use-new-parser
			--expected-output=${CMAKE_CURRENT_SOURCE_DIR}/${name}.log
			${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci)
	if(TEST_CHECK_OUTPUT_EXPECT_FAIL)
		set_tests_properties("${test_name}" PROPERTIES LABELS "EXPECT_FAIL")
	endif()
endmacro(test_check_output)

add_subdirectory(Alias)
add_subdirectory(Assert)
add_subdirectory(Auto)
add_subdirectory(BindingRef)
add_subdirectory(Cast)
add_subdirectory(Class)
add_subdirectory(Conditional)
add_subdirectory(Const)
add_subdirectory(CTypes)
add_subdirectory(Datatype)
add_subdirectory(Defaults)
add_subdirectory(Destructor)
add_subdirectory(EvaluationOrder)
add_subdirectory(Exception)
add_subdirectory(Function)
add_subdirectory(Interface)
add_subdirectory(Liveness)
add_subdirectory(Loop)
add_subdirectory(Lval)
add_subdirectory(Move)
add_subdirectory(Primitives)
add_subdirectory(Ref)
add_subdirectory(ScopeExit)
add_subdirectory(SizeOf)
add_subdirectory(StaticRef)
add_subdirectory(Switch)
add_subdirectory(Templates)
add_subdirectory(ValueTemplates)

