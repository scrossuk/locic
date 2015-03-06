# Compiler tests for programs that are expected to
# successfully compile and produce specific output.

macro(test_check_output name)
	add_test(NAME TestCheckOutput${name}
		COMMAND ${CMAKE_BINARY_DIR}/tools/loci-test --test-name TestCheckOutput${name} --expected-output=${CMAKE_CURRENT_SOURCE_DIR}/${name}.log ${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci)
endmacro(test_check_output)

add_subdirectory(Assert)
add_subdirectory(Auto)
add_subdirectory(BindingRef)
add_subdirectory(Cast)
add_subdirectory(Class)
add_subdirectory(Conditional)
add_subdirectory(Const)
add_subdirectory(Datatype)
add_subdirectory(Defaults)
add_subdirectory(Exception)
add_subdirectory(Interface)
add_subdirectory(Loop)
add_subdirectory(Lval)
add_subdirectory(Move)
add_subdirectory(Ref)
add_subdirectory(ScopeExit)
add_subdirectory(SizeOf)
add_subdirectory(StaticRef)
add_subdirectory(Switch)
add_subdirectory(Templates)
add_subdirectory(TypeAlias)

