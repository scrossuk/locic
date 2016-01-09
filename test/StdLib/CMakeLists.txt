# Standard library tests for programs that are expected to
# successfully compile and produce specific output.

set(STD_ALL_API_FILES
	${STD_CONCURRENCY_API_FILES}
	${STD_CONTAINER_API_FILES}
	${STD_EVENT_API_FILES}
	${STD_MEMORY_API_FILES}
	${STD_STRING_API_FILES}
	)

set(STD_ALL_BINARY_FILES
	${STD_CONCURRENCY_BINARY_FILES}
	${STD_CONTAINER_BINARY_FILES}
	${STD_EVENT_BINARY_FILES}
	${STD_MEMORY_BINARY_FILES}
	${STD_STRING_BINARY_FILES}
)

macro(test_stdlib_check_output name)
	add_test(NAME TestStdLibCheckOutput${name}
		COMMAND ${CMAKE_BINARY_DIR}/tools/loci-test --test-name TestStdLibCheckOutput${name}
			--expected-output=${CMAKE_CURRENT_SOURCE_DIR}/${name}.log
			${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci ${STD_ALL_API_FILES}
			--dependency-modules ${STD_ALL_BINARY_FILES}
	)
endmacro(test_stdlib_check_output)

add_subdirectory(Concurrency)
add_subdirectory(Container)
add_subdirectory(Memory)
add_subdirectory(String)

